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

from notebook.connectors.base import Notebook
from notebook.converters.exceptions import NotebookConvertException


LOG = logging.getLogger(__name__)


def is_jupyter(data):
  is_jupyter_format = False
  try:
    json_data = json.loads(data)

    if 'nbformat' in json_data:
      nb_keys = set(['metadata', 'nbformat', 'nbformat_minor'])

      if json_data['nbformat'] == 4:
        nb_keys.add('cells')
      else:
        nb_keys.add('worksheets')
      # TODO: Add support for other iPython versions

      if set(json_data.keys()) == nb_keys:
        is_jupyter_format = True
  except ValueError:
    pass
  return is_jupyter_format


class StatementRaw(unicode):
  pass


class JupyterConverter(object):

  def __init__(self, data):
    self.source_data = json.loads(data)

  @property
  def name(self):
    if self.source_data['nbformat'] == 4:
      return self.source_data['metadata']['display_name']
    else:
      return self.source_data['metadata']['name']

  @property
  def cells(self):
    cells = None
    if self.source_data['nbformat'] == 4:
      cells = self.source_data['cells']
    elif self.source_data['nbformat'] == 3:
      cells = self.source_data['worksheets'][0]['cells']
    # TODO: Add support for other iPython versions
    return cells

  def convert_to_notebook(self):
    try:
      notebook = Notebook.get_default_notebook()
      notebook['name'] = self.name
      for cell in self.cells:
        snippet = self._cell_to_snippet(cell)
        notebook['snippets'].append(snippet)
      return notebook
    except Exception, e:
      raise NotebookConvertException('Failed to convert Jupyter file to notebook format: %s' % str(e))

  def _cell_to_snippet(self, cell):
    snippet = Notebook.get_default_snippet()

    if cell['cell_type'] == 'code' and cell['language'] == 'python':
      snippet['type'] = 'pyspark'
      if self.source_data['nbformat'] == 4:
        snippet['statement_raw'] = cell['source']
      else:
        snippet['statement_raw'] = StatementRaw(cell['input'])
    elif cell['cell_type'] == 'heading':
      snippet['type'] = 'markdown'
      statement_raw = cell['source']
      if cell['level'] == 1:
        statement_raw += "\n===================="
      elif cell['level'] == 2:
        statement_raw += "\n--------------------"
      else:
        statement_raw = "### " + statement_raw
      snippet['statement_raw'] = StatementRaw(statement_raw)
    else:
      snippet['type'] = 'markdown'
      snippet['statement_raw'] = StatementRaw(cell['source'])

    return snippet
