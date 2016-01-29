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
import json
import tempfile
import StringIO
import zipfile

from django.contrib.auth.models import Group, User
from django.core import management
from django.db import transaction
from django.http import HttpResponse
from django.shortcuts import redirect
from django.utils.translation import ugettext as _
from django.views.decorators.http import require_POST

from beeswax.models import SavedQuery
from desktop.lib.django_util import JsonResponse
from desktop.lib.exceptions_renderable import PopupException
from desktop.lib.export_csvxls import make_response
from desktop.lib.i18n import smart_str, force_unicode
from desktop.models import Document2, Document, Directory, DocumentTag, FilesystemException, import_saved_beeswax_query


LOG = logging.getLogger(__name__)


def api_error_handler(func):
  def decorator(*args, **kwargs):
    response = {}

    try:
      return func(*args, **kwargs)
    except Exception, e:
      LOG.exception('Error running %s' % func)
      response['status'] = -1
      response['message'] = force_unicode(str(e))
    finally:
      if response:
        return JsonResponse(response)

  return decorator


@api_error_handler
def get_documents(request):
  """
  Returns all documents and directories found for the given uuid or path and current user.
  Optional params:
    page=<n>    - Controls pagination. Defaults to 1.
    limit=<n>   - Controls limit per page. Defaults to all.
    type=<type> - Show documents of given type(s) (directory, query-hive, query-impala, query-mysql, etc). Default to all.
    sort=<key>  - Sort by the attribute <key>, which is one of:
                    "name", "type", "owner", "last_modified"
                  Accepts the form "-last_modified", which sorts in descending order.
                  Default to "-last_modified".
    text=<frag> - Search for fragment "frag" in names and descriptions.
  """
  path = request.GET.get('path', '/')
  uuid = request.GET.get('uuid')

  if uuid:
    document = Document2.objects.get_by_uuid(uuid)
  else:  # Find by path
    document = Document2.objects.get_by_path(user=request.user, path=path)

  # Check if user has read permissions
  document.can_read_or_exception(request.user)

  # Get querystring filters if any
  page = int(request.GET.get('page', 1))
  limit = int(request.GET.get('limit', 0))
  type_filters = request.GET.getlist('type', None)
  sort = request.GET.get('sort', '-last_modified')
  search_text = request.GET.get('text', None)

  # Get children documents if this is a directory
  children = None
  count = 0
  if document.is_directory:
    directory = Directory.objects.get(id=document.id)
    children = directory.documents(types=type_filters, search_text=search_text, order_by=sort)
    count = children.count()

  # Paginate
  if children and limit > 0:
    offset = (page - 1) * limit
    last = offset + limit
    children = children.all()[offset:last]

  return JsonResponse({
      'document': document.to_dict(),
      'parent': document.parent_directory.to_dict() if document.parent_directory else None,
      'children': [doc.to_dict() for doc in children] if children else [],
      'page': page,
      'limit': limit,
      'count': count,
      'types': type_filters,
      'sort': sort,
      'text': search_text
  })


@api_error_handler
def get_document(request):
  if request.GET.get('id'):
    doc = Document2.objects.get(id=request.GET['id'])
  else:
    doc = Document2.objects.get_by_uuid(uuid=request.GET['uuid'])

  # Check if user has read permissions
  doc.can_read_or_exception(request.user)

  return JsonResponse(doc.to_dict())


@api_error_handler
@require_POST
def move_document(request):
  source_doc_uuid = json.loads(request.POST.get('source_doc_uuid'))
  destination_doc_uuid = json.loads(request.POST.get('destination_doc_uuid'))

  if not source_doc_uuid or not destination_doc_uuid:
    raise PopupException(_('move_document requires source_doc_uuid and destination_doc_uuid'))

  source = Directory.objects.get_by_uuid(uuid=source_doc_uuid)
  destination = Directory.objects.get_by_uuid(uuid=destination_doc_uuid)

  # Check if user has write permissions for both source and destination
  source.can_write_or_exception(request.user)
  destination.can_write_or_exception(request.user)

  doc = source.move(destination, request.user)

  return JsonResponse({
    'status': 0,
    'document': doc.to_dict()
  })


@api_error_handler
@require_POST
def create_directory(request):
  parent_uuid = json.loads(request.POST.get('parent_uuid'))
  name = json.loads(request.POST.get('name'))

  if not parent_uuid or not name:
    raise PopupException(_('create_directory requires parent_uuid and name'))

  parent_dir = Directory.objects.get_by_uuid(uuid=parent_uuid)

  # Check if user has write permissions for parent directory
  parent_dir.can_write_or_exception(request.user)

  directory = Directory.objects.create(name=name, owner=request.user, parent_directory=parent_dir)

  return JsonResponse({
      'status': 0,
      'directory': directory.to_dict()
  })


@api_error_handler
@require_POST
def delete_document(request):
  """
  Accepts a uuid and optional skip_trash parameter

  (Default) skip_trash=false, flags a document as trashed
  skip_trash=true, deletes it permanently along with any history dependencies

  If directory and skip_trash=false, all dependencies will also be flagged as trash
  If directory and skip_trash=true, directory must be empty (no dependencies)
  """
  uuid = json.loads(request.POST.get('uuid'))
  skip_trash = json.loads(request.POST.get('skip_trash', 'false'))

  if not uuid:
    raise PopupException(_('delete_document requires uuid'))

  document = Document2.objects.get_by_uuid(uuid=uuid)

  # Check if user has write permissions for given document
  document.can_write_or_exception(request.user)

  if skip_trash:
    # TODO: check if document is in the .Trash folder, if not raise exception
    if document.is_directory and document.has_children:
      raise PopupException(_('Directory is not empty'))
    document.delete()
  else:
    document.trash()  # TODO: get number of docs trashed

  return JsonResponse({
      'status': 0,
  })


@api_error_handler
@require_POST
def share_document(request):
  """
  Set who else or which other group can interact with the document.

  Example of input: {'read': {'user_ids': [1, 2, 3], 'group_ids': [1, 2, 3], 'all': false}}
  """
  perms_dict = json.loads(request.POST.get('data'))
  uuid = json.loads(request.POST.get('uuid'))

  if not uuid or not perms_dict:
    raise PopupException(_('share_document requires uuid and perms_dict'))

  doc = Document2.objects.get_by_uuid(uuid=uuid)

  for name, perm in perms_dict.iteritems():
    users = groups = None
    if perm.get('user_ids'):
      users = User.objects.in_bulk(perm.get('user_ids'))
    else:
      users = []

    if perm.get('group_ids'):
      groups = Group.objects.in_bulk(perm.get('group_ids'))
    else:
      groups = []

    all = perm.get('all', False)

    doc = doc.share(request.user, name=name, users=users, groups=groups, all=all)

  return JsonResponse({
    'status': 0,
    'document': doc.to_dict()
  })


def export_documents(request):
  if request.GET.get('documents'):
    selection = json.loads(request.GET.get('documents'))
  else:
    selection = json.loads(request.POST.get('documents'))

  # If non admin, only export documents the user owns
  docs = Document2.objects
  if not request.user.is_superuser:
    docs = docs.filter(owner=request.user)
  docs = docs.filter(id__in=selection).order_by('-id')
  doc_ids = docs.values_list('id', flat=True)

  f = StringIO.StringIO()

  if doc_ids:
    doc_ids = ','.join(map(str, doc_ids))
    management.call_command('dumpdata', 'desktop.Document2', primary_keys=doc_ids, indent=2, use_natural_keys=True, verbosity=2, stdout=f)

  if request.GET.get('format') == 'json':
    return JsonResponse(f.getvalue(), safe=False)
  elif request.GET.get('format') == 'zip':
    zfile = zipfile.ZipFile(f, 'w')
    zfile.writestr("hue.json", f.getvalue())
    for doc in docs:
      if doc.type == 'notebook':
        try:
          from spark.models import Notebook
          zfile.writestr("notebook-%s-%s.txt" % (doc.name, doc.id), smart_str(Notebook(document=doc).get_str()))
        except Exception, e:
          print e
          LOG.exception(e)
    zfile.close()
    response = HttpResponse(content_type="application/zip")
    response["Content-Length"] = len(f.getvalue())
    response['Content-Disposition'] = 'attachment; filename="hue-documents.zip"'
    response.write(f.getvalue())
    return response
  else:
    return make_response(f.getvalue(), 'json', 'hue-documents')


def import_documents(request):
  if request.FILES.get('documents'):
    documents = request.FILES['documents'].read()
  else:
    documents = json.loads(request.POST.get('documents'))

  documents = json.loads(documents)
  docs = []

  for doc in documents:
    if not request.user.is_superuser:
      doc['fields']['owner'] = [request.user.username]
    owner = doc['fields']['owner'][0]

    doc['fields']['tags'] = []

    # TODO: Check if this should be replaced by get_by_uuid
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
    return JsonResponse({'message': smart_str(e)})

  Document.objects.sync()

  if request.POST.get('redirect'):
    return redirect(request.POST.get('redirect'))
  else:
    return JsonResponse({'message': stdout.getvalue()})


def _convert_documents(user):
  """
  Given a user, converts any existing Document objects to Document2 objects
  """
  from beeswax.models import HQL, IMPALA, RDBMS

  with transaction.atomic():
    # If user does not have a home directory, we need to create one and import any orphan documents to it
    Document2.objects.create_user_directories(user)

    docs = Document.objects.get_docs(user, SavedQuery).filter(owner=user).filter(extra__in=[HQL, IMPALA, RDBMS])

    imported_tag = DocumentTag.objects.get_imported2_tag(user=user)

    docs = docs.exclude(tags__in=[
        DocumentTag.objects.get_trash_tag(user=user),  # No trashed docs
        DocumentTag.objects.get_history_tag(user=user),  # No history yet
        DocumentTag.objects.get_example_tag(user=user),  # No examples
        imported_tag  # No already imported docs
    ])

    root_doc, created = Directory.objects.get_or_create(name='', owner=user)
    imported_docs = []

    for doc in docs:
      if doc.content_object:
        try:
          notebook = import_saved_beeswax_query(doc.content_object)
          data = notebook.get_data()
          notebook_doc = Document2.objects.create(name=data['name'], type=data['type'], owner=user, data=notebook.get_json())

          doc.add_tag(imported_tag)
          doc.save()
          imported_docs.append(notebook_doc)
        except Exception, e:
          raise e

    if imported_docs:
      root_doc.children.add(*imported_docs)
