#!/usr/bin/env python
# Licensed to Cloudera, Inc. under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  Cloudera, Inc. licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import re

from desktop.lib.exceptions_renderable import PopupException
from desktop.lib.i18n import force_unicode

from beeswax import data_export
from beeswax.design import hql_query
from beeswax import conf as beeswax_conf
from beeswax.models import QUERY_TYPES, HiveServerQueryHandle, QueryHistory, HiveServerQueryHistory
from beeswax.server import dbms
from beeswax.server.dbms import get_query_server_config, QueryServerException
from beeswax.views import _parse_out_hadoop_jobs

from notebook.connectors.base import Api, QueryError, QueryExpired


LOG = logging.getLogger(__name__)


def query_error_handler(func):
  def decorator(*args, **kwargs):
    try:
      return func(*args, **kwargs)
    except QueryServerException, e:
      message = force_unicode(str(e))
      if 'Invalid query handle' in message or 'Invalid OperationHandle' in message:
        raise QueryExpired(e)
      else:
        raise QueryError(message)
  return decorator


class HS2Api(Api):

  def _get_handle(self, snippet):
    snippet['result']['handle']['secret'], snippet['result']['handle']['guid'] = HiveServerQueryHandle.get_decoded(snippet['result']['handle']['secret'], snippet['result']['handle']['guid'])
    return HiveServerQueryHandle(**snippet['result']['handle'])

  def _get_db(self, snippet):
    if snippet['type'] == 'hive':
      name = 'beeswax'
    elif snippet['type'] == 'impala':
      name = 'impala'
    else:
      name = 'spark-sql'

    return dbms.get(self.user, query_server=get_query_server_config(name=name))

  def execute(self, notebook, snippet):
    db = self._get_db(snippet)
    query = hql_query(snippet['statement'], QUERY_TYPES[0])

    try:
      handle = db.client.query(query)
    except QueryServerException, ex:
      raise QueryError(ex.message)

    # All good
    server_id, server_guid  = handle.get()
    return {
        'secret': server_id,
        'guid': server_guid,
        'operation_type': handle.operation_type,
        'has_result_set': handle.has_result_set,
        'modified_row_count': handle.modified_row_count,
        'log_context': handle.log_context
    }

  @query_error_handler
  def check_status(self, notebook, snippet):
    response = {}
    db = self._get_db(snippet)

    handle = self._get_handle(snippet)
    operation = db.get_operation_status(handle)
    status = HiveServerQueryHistory.STATE_MAP[operation.operationState]

    if status.index in (QueryHistory.STATE.failed.index, QueryHistory.STATE.expired.index):
      raise QueryError(operation.errorMessage)

    response['status'] = 'running' if status.index in (QueryHistory.STATE.running.index, QueryHistory.STATE.submitted.index) else 'available'

    return response

  @query_error_handler
  def fetch_result(self, notebook, snippet, rows, start_over):
    db = self._get_db(snippet)

    handle = self._get_handle(snippet)
    results = db.fetch(handle, start_over=start_over, rows=rows)

    # No escaping...
    return {
        'has_more': results.has_more,
        'data': list(results.rows()),
        'meta': [{
          'name': column.name,
          'type': column.type,
          'comment': column.comment
        } for column in results.data_table.cols()],
        'type': 'table'
    }

  @query_error_handler
  def fetch_result_metadata(self):
    pass

  @query_error_handler
  def cancel(self, notebook, snippet):
    db = self._get_db(snippet)

    handle = self._get_handle(snippet)
    db.cancel_operation(handle)
    return {'status': 0}

  @query_error_handler
  def get_log(self, notebook, snippet, startFrom=None, size=None):
    db = self._get_db(snippet)

    handle = self._get_handle(snippet)
    return db.get_log(handle, start_over=startFrom == 0)

  def download(self, notebook, snippet, format):
    try:
      db = self._get_db(snippet)
      handle = self._get_handle(snippet)
      return data_export.download(handle, format, db)
    except Exception, e:
      LOG.exception('error downloading notebook')

      if not hasattr(e, 'message') or not e.message:
        message = e
      else:
        message = e.message
      raise PopupException(message, detail='')

  def _progress(self, snippet, logs):
    if snippet['type'] == 'hive':
      match = re.search('Total jobs = (\d+)', logs, re.MULTILINE)
      total = (int(match.group(1)) if match else 1) * 2

      started = logs.count('Starting Job')
      ended = logs.count('Ended Job')

      return int((started + ended) * 100 / total)
    elif snippet['type'] == 'impala':
      match = re.search('(\d+)% Complete', logs, re.MULTILINE)
      return int(match.group(1)) if match else 0
    else:
      return 50

  @query_error_handler
  def close_statement(self, snippet):
    if snippet['type'] == 'impala':
      from impala import conf as impala_conf

    if (snippet['type'] == 'hive' and beeswax_conf.CLOSE_QUERIES.get()) or (snippet['type'] == 'impala' and impala_conf.CLOSE_QUERIES.get()):
      db = self._get_db(snippet)

      handle = self._get_handle(snippet)
      db.close_operation(handle)
      return {'status': 0}
    else:
      return {'status': -1}  # skipped

  def _get_jobs(self, log):
    return _parse_out_hadoop_jobs(log)
