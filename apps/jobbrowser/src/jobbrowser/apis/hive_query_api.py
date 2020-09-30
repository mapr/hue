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
from builtins import filter

import logging
import re

from datetime import datetime
from django.utils.translation import ugettext as _

from desktop.lib.exceptions_renderable import PopupException
from desktop.lib.python_util import current_ms_from_utc

from desktop.lib.rest.http_client import HttpClient
from desktop.lib.rest.resource import Resource

from jobbrowser.apis.base_api import Api
from jobbrowser.models import HiveQuery
from jobbrowser.conf import DAS_SERVER_URL


LOG = logging.getLogger(__name__)


class HiveQueryApi(Api):

  def __init__(self, user, cluster=None):
    self.user = user
    self.cluster = cluster
    self.api = HiveQueryClient()
    self.headers = {'X-Requested-By': 'das'}

  def apps(self, filters):
    queries = self.api.get_queries(limit=100)

    apps = {
      "queries": [{
          "details": None,
          "dags": [],
          "id": query.query_id,
          "queryId": query.query_id,
          "startTime": query.start_time,
          "query": query.query.replace('\r\n', ' ')[:60] + ('...' if len(query.query) > 60 else ''),
          "highlightedQuery": None,
          "endTime": query.end_time,
          "elapsedTime": query.elapsed_time,
          "status": query.status,
          "queueName": query.queue_name,
          "userId": query.user_id,
          "requestUser": query.request_user,
          "cpuTime": query.cpu_time,
          "physicalMemory": query.physical_memory,
          "virtualMemory": query.virtual_memory,
          "dataRead": query.data_read,
          "dataWritten": query.data_written,
          "operationId": query.operation_id,
          "clientIpAddress": query.client_ip_address,
          "hiveInstanceAddress": query.hive_instance_address,
          "hiveInstanceType": query.hive_instance_type,
          "sessionId": query.session_id,
          "logId": query.log_id,
          "threadId": query.thread_id,
          "executionMode": query.execution_mode,
          "tablesRead": query.tables_read,
          "tablesWritten": query.tables_written,
          "databasesUsed": query.databases_used,
          "domainId": query.domain_id,
          "llapAppId": query.llap_app_id,
          "usedCBO": query.used_cbo,
          "processed": query.processed,
          "createdAt": query.created_at
        }
        for query in queries
      ],
      "meta": {
          "limit": 25,
          "offset": 0,
          "size": self.api.get_query_count()
        }
    }

    return apps

  def app(self, appid):
    query = self.api.get_query(query_id=appid)

    if not query:
      raise PopupException(_('Could not find query id %s' % appid))

    params = {
      'extended': 'true',
      'queryId': query.query_id
    }

    client = HttpClient(DAS_SERVER_URL.get())
    resource  = Resource(client)
    app = resource.get('api/hive/query', params=params, headers=self.headers)

    return app

  def action(self, appid, action):
    message = {'message': '', 'status': 0}

    return message;

  def logs(self, appid, app_type, log_name=None, is_embeddable=False):
    return {'logs': ''}

  def profile(self, appid, app_type, app_property, app_filters):
    message = {'message': '', 'status': 0}

    return message;

  def _api_status(self, status):
    if status == 'SUCCESS':
      return 'SUCCEEDED'
    elif status == 'EXCEPTION':
      return 'FAILED'
    elif status == 'RUNNING':
      return 'RUNNING'
    else:
      return 'PAUSED'


class HiveQueryClient():

  def get_query_count(self):
    return HiveQuery.objects.using('query').count()

  def get_queries(self, limit=100):
    return HiveQuery.objects.using('query').order_by('-id')[:limit]

  def get_query(self, query_id):
    return HiveQuery.objects.using('query').get(query_id=query_id)

  def get_query_analysis(self, query_id): pass

  # EXPLAIN with row count
  # CBO COST
  # VECTORIZATION?
