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

from desktop.models import Document2, Document

from notebook.converters.exceptions import NotebookConvertException
from notebook.converters.jupyter import is_jupyter, JupyterConverter, StatementRaw


LOG = logging.getLogger(__name__)


class NotebookEncoder(json.JSONEncoder):

  # monkeypatch json.encoder module
  for name in ['encode_basestring', 'encode_basestring_ascii']:
    def encode(o, _encode=getattr(json.encoder, name)):
        return o if isinstance(o, StatementRaw) else _encode(o)
    setattr(json.encoder, name, encode)


class NotebookConverter(object):

  def __init__(self, owner, filepath, data, **options):
    self.owner = owner
    self.filename = self._get_filename(filepath)
    self.source_data = data
    self.options = options


  @property
  def is_notebook(self):
    is_notebook_format = False
    if self.filename.endswith('.json'):
      try:
        # TODO: Actually validate the JSON format for Hue Notebook format
        json.loads(self.source_data)
        is_notebook_format = True
      except ValueError:
        pass
    return is_notebook_format


  def convert_to_notebook_document(self):
    document_json = None
    if self.is_notebook:
      document_json = self.source_data
    else:
      if is_jupyter(self.source_data):
        converter = JupyterConverter(self.source_data)
        notebook = converter.convert_to_notebook()
      else:
        raise NotebookConvertException('Cannot convert unknown source format %s' % self.filename)

      if notebook:
        document = self._create_document_from_notebook(notebook)
        document_json = self._serialize_document_to_json(document)

    return document_json


  def _get_filename(self, filepath):
    return filepath.strip('/').split()[0]


  def _create_document_from_notebook(self, notebook):
    notebook_doc = Document2.objects.create(name=notebook['name'], type='notebook', owner=self.owner)
    Document.objects.link(notebook_doc, owner=notebook_doc.owner, name=notebook_doc.name, description=notebook_doc.description, extra='notebook')
    notebook_doc1 = notebook_doc.doc.get()
    notebook_doc.update_data(notebook)
    notebook_doc.name = notebook_doc1.name = notebook['name']
    notebook_doc.description = notebook_doc1.description = notebook['description']
    return notebook_doc


  def _serialize_document_to_json(self, document):
    doc_dict = [{
      'pk': None,
      'model': 'desktop.document2',
      'fields': {
        'uuid': document.uuid,
        'extra': document.extra,
        'type': document.type,
        'description': document.description,
        'tags': [],
        'is_history': document.is_history,
        'last_modified': document.last_modified.isoformat(), #'2015-09-23T17:16:54.675',  # TODO: Fix last modified import JSON TZ to MySQL
        'version': document.version,
        'owner': [document.owner.username],
        'dependencies': [],
        'data': document.data_dict,
        'name': document.name
      }
    }]
    return json.dumps(doc_dict, cls=NotebookEncoder)
