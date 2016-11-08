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

import json
import logging

from desktop.lib.django_util import JsonResponse
from django.utils.translation import ugettext as _
from django.views.decorators.http import require_POST

from beeswax.api import error_handler
from beeswax.common import HIVE_IDENTIFER_REGEX
from beeswax.server import dbms as beeswax_dbms

from impala import dbms as impala_dbms


LOG = logging.getLogger(__name__)


KUDU_PRIMITIVE_TYPES = ("string", "tinyint", "smallint", "int", "bigint", "boolean", "float", "double", "timestamp")
KUDU_PARTITION_TYPES = ("hash", "range")


class CreateTableValidationException(Exception):
  pass


@require_POST
@error_handler
def create_internal_kudu_table(request):
  query_server = impala_dbms.get_query_server_config()
  db = beeswax_dbms.get(request.user, query_server=query_server)

  response = {'status': -1, 'message': ''}

  database = json.loads(request.POST.get('database', ''))
  table_name = json.loads(request.POST.get('table', ''))
  table_description = json.loads(request.POST.get('description', ''))
  columns = json.loads(request.POST.get('columns', '[]'))
  partitions = json.loads(request.POST.get('partitions', '[]'))
  #properties = json.loads(request.POST.get('properties', '[]'))
  properties = [{'key': 'kudu.master_addresses', 'value': 'jennykim-1.vpc.cloudera.com:7051'}]

  _validate_kudu_table(database, table_name, table_description)
  _validate_kudu_columns(columns)
  _validate_kudu_partitions(columns, partitions)
  # TODO: validate and add custom TBLPROPERTIES

  statement = "CREATE TABLE %(table)s (%(columns)s, PRIMARY KEY (%(pk)s)) DISTRIBUTE BY %(partitions)s STORED AS KUDU" % {
      'table': table_name,
      'columns': ", ".join("%s %s" % (col['name'], col['type'].upper()) for col in columns),
      'pk': ','.join([col['name'] for col in columns if col['pk']]),
      'partitions': _format_partitions(partitions)
    }

  if properties:
    statement += " TBLPROPERTIES (%s)" % ", ".join("'%s' = '%s'" % (prop['key'], prop['value']) for prop in properties)

  db.execute_statement(statement)
  # TODO: add table data to response
  response['status'] = 0
  return JsonResponse(response)


def _format_partitions(partitions):
  partition_specs = []
  for partition in partitions:
    if partition['type'].lower() == 'hash':
      partition_specs.append('HASH (%s) INTO %d BUCKETS' % (partition['column'], partition['num_buckets']))
    # TODO: add range handling
  return ", ".join(partition_specs)


@require_POST
@error_handler
def validate_kudu_table(request):
  response = {'status': -1, 'message': ''}

  database = json.loads(request.POST.get('database', ''))
  table_name = json.loads(request.POST.get('table', ''))
  table_description = json.loads(request.POST.get('description', ''))

  _validate_kudu_table(database, table_name, table_description)

  response['status'] = 0
  return JsonResponse(response)


def _validate_kudu_table(database, table_name, table_description=None):
  """
  Checks if a new Kudu table is valid and does not already exist for the given database
  """
  if not database:
    raise CreateTableValidationException(_('Database name is required.'))

  if not table_name:
    raise CreateTableValidationException(_('Table name is required.'))

  if not HIVE_IDENTIFER_REGEX.match(table_name):
    raise CreateTableValidationException(_('Table name "%s" is invalid.') % table_name)

  # TODO: Validate that table does not already exist
  #query_server = impala_dbms.get_query_server_config()
  #db = beeswax_dbms.get(request.user, query_server=query_server)

  # TODO: Validate table description


@require_POST
@error_handler
def validate_kudu_columns(request):
  response = {'status': -1, 'message': ''}

  columns = json.loads(request.POST.get('columns', '[]'))

  _validate_kudu_columns(columns)

  response['status'] = 0
  return JsonResponse(response)


def _validate_kudu_columns(columns):
  """
  Checks if the given list of column objects with name, type and PK flag are valid
  e.g. - [
    {"name": "id", "type": "int", "pk": True}
    {"name": "name", "type": "string", "pk": False}
  ]
  """
  if not columns:
    raise CreateTableValidationException(_('Columns must contain a list of column objects with name, type, and pk.'))

  for column in columns:
    if not isinstance(column, dict) or not all(key in column for key in ['name', 'type', 'pk']):
      raise CreateTableValidationException(_('Column object must be a dict with name, type, and pk keys.'))

    if not HIVE_IDENTIFER_REGEX.match(column['name']):
      raise CreateTableValidationException(_('Column name "%s" is invalid.') % column['name'])

    if not column['type'] or column['type'].lower() not in KUDU_PRIMITIVE_TYPES:
      raise CreateTableValidationException(_('Column type "%s" is not valid, available types are: %s') %
                                             (column['type'], ', '.join(KUDU_PRIMITIVE_TYPES)))

    if column['pk'] and column['type'].lower() in ('boolean', 'float', 'double'):
      raise CreateTableValidationException(_('Primary key may not be a boolean or floating-point type.'))

  if not any(column['pk'] for column in columns):
    raise CreateTableValidationException(_('Must declare at least one column as the primary key.'))


@require_POST
@error_handler
def validate_kudu_partitions(request):
  response = {'status': -1, 'message': ''}

  partitions = json.loads(request.POST.get('partitions', '[]'))

  _validate_kudu_partitions(partitions)

  response['status'] = 0
  return JsonResponse(response)


def _validate_kudu_partitions(columns, partitions):
  """
  Checks if the given list of partition objects with type, column(s) and other attributes are valid
  e.g. - [
    {"type": "hash", "column": "id", "num_buckets": 16}
    {"type": "range", "column": "name", "split_rows": ["abc", "def"]}
  ]
  """
  if not columns:
    raise CreateTableValidationException(_('Columns must contain a list of column objects with name, type, and pk.'))

  if not partitions:
    raise CreateTableValidationException(_('Partitions must contain a list of partition objects with type, column ' +
                                           'and either num_buckets (hash) or split_rows (range).'))

  for partition in partitions:
    if not partition['type'] or partition['type'].lower() not in KUDU_PARTITION_TYPES:
      raise CreateTableValidationException(_('Partition type "%s" is not valid, available types are: %s') %
                                           (partition['type'], ', '.join(KUDU_PARTITION_TYPES)))

    if partition['type'].lower() == 'hash':
      if not partition['column']:
        raise CreateTableValidationException(_('Hash partition requires a selected primary key column.'))

      column = next((column for column in columns if column['name'].lower() == partition['column'].lower()), None)

      if not column:
        raise CreateTableValidationException(_('Column named "%s" not defined.') % partition['column'])

      if not column['pk']:
        raise CreateTableValidationException(_('Hash columns must be a primary key column.'))

    # TODO: validate range partition
