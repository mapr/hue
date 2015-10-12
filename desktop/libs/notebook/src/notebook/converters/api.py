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
import StringIO
import tempfile

from django.core import management
from django.http import Http404
from django.utils.translation import ugettext as _

from desktop.lib.django_util import JsonResponse
from desktop.lib.i18n import force_unicode, smart_str
from desktop.models import Document2, Document

from notebook.converters.convert import NotebookConverter
from notebook.converters.github_client import GithubClient, GithubImportException

LOG = logging.getLogger(__name__)


class NotebookImportException(Exception):
  pass


def error_handler(view_fn):
  def decorator(request, *args, **kwargs):
    try:
      return view_fn(request, *args, **kwargs)
    except Http404, e:
      raise e
    except Exception, e:
      LOG.exception('Error in %s' % view_fn)
      message = str(e)
      response = {
        'status': -1,
        'message': message,
      }
      return JsonResponse(response)
  return decorator


@error_handler
def fetch_github(request):
  response = {'status': -1}

  api = GithubClient()

  response['url'] = url = request.GET.get('url')
  response['filepath'] = filepath = request.GET.get('filepath')

  if url and filepath:
    username, repo = GithubClient.get_username_and_repo(url)
    response['username'] = username
    response['repo'] = repo

    response['status'] = 0
    response['content'] = api.get_file_contents(username, repo, filepath)
  else:
    response['message'] = _('Github fetch requires repository URL and path to file.')

  return JsonResponse(response)


@error_handler
def import_github(request):
  response = {'status': -1}

  api = GithubClient()

  response['url'] = url = request.GET.get('url')
  response['filepath'] = filepath = request.GET.get('filepath')

  if url and filepath:
    username, repo = GithubClient.get_username_and_repo(url)
    response['username'] = username
    response['repo'] = repo

    content = api.get_file_contents(username, repo, filepath)
    if content:
      converter = NotebookConverter(request.user, filepath, content)
      document = converter.convert_to_notebook_document()
      notebook = json.loads(document)
      response.update(_import_notebooks(request.user, notebook))
    else:
      response['message'] = _('Github file contained empty contents: %s/%s/%s') % (username, repo, filepath)
  else:
    response['message'] = _('Github import requires repository URL and path to file.')

  return JsonResponse(response)


# TODO: This is mostly repetitive code that is also in desktop/api2.py, we should consolidate
def _import_notebooks(user, notebooks):
  docs = []

  for doc in notebooks:
    # Reset owner and tags
    doc['fields']['owner'] = [user.username]
    doc['fields']['tags'] = []
    owner = doc['fields']['owner'][0]

    if Document2.objects.filter(uuid=doc['fields']['uuid'], owner__username=owner).exists():
      doc['pk'] = Document2.objects.get(uuid=doc['fields']['uuid'], owner__username=owner).pk
    else:
      doc['pk'] = None

    docs.append(doc)

  f = tempfile.NamedTemporaryFile(mode='w+', suffix='.json')
  f.write(json.dumps(docs))


  f.flush()

  stdout = StringIO.StringIO()
  try:
    management.call_command('loaddata', f.name, stdout=stdout)
  except Exception, e:
    raise NotebookImportException(e)

  Document.objects.sync()

  return {
    'status': 0,
    'message': stdout.getvalue()
  }
