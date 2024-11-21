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

import os
import logging
import posixpath
from io import BytesIO as string_io

from django.http import HttpResponse

from desktop.lib.django_util import JsonResponse
from desktop.lib import fsmanager
from desktop.lib.i18n import smart_unicode
from filebrowser.conf import (
  MAX_FILE_SIZE_UPLOAD_LIMIT,
  RESTRICT_FILE_EXTENSIONS,
)
from filebrowser.lib.rwx import filetype, rwx
from filebrowser.views import (
  stat_absolute_path,
)

from azure.abfs.__init__ import get_home_dir_for_abfs
from aws.s3.s3fs import get_s3_home_directory


LOG = logging.getLogger(__name__)


def error_handler(view_fn):
  def decorator(*args, **kwargs):
    response = {}
    try:
      return view_fn(*args, **kwargs)
    except Exception as e:
      LOG.exception('Error running %s' % view_fn)
      response['status'] = -1
      response['message'] = smart_unicode(e)
    return JsonResponse(response)
  return decorator


@error_handler
def get_filesystems(request):
  response = {}

  filesystems = {}
  for k in fsmanager.get_filesystems(request.user):
    filesystems[k] = True

  response['status'] = 0
  response['filesystems'] = filesystems

  return JsonResponse(response)

# TODO: Improve error response further with better context -- Error UX Phase 2
def api_error_handler(view_fn):
  """
  Decorator to handle exceptions and return a JSON response with an error message.
  """

  def decorator(*args, **kwargs):
    try:
      return view_fn(*args, **kwargs)
    except Exception as e:
      LOG.exception(f'Error running {view_fn.__name__}: {str(e)}')
      return JsonResponse({'error': str(e)}, status=500)

  return decorator

@error_handler
def get_filesystems_with_home_dirs(request): # Using as a public API only for now
  filesystems = []
  user_home_dir = ''

  for fs in fsmanager.get_filesystems(request.user):
    if fs == 'hdfs':
      user_home_dir = request.user.get_home_directory()
    elif fs == 's3a':
      user_home_dir = get_s3_home_directory(request.user)
    elif fs == 'abfs':
      user_home_dir = get_home_dir_for_abfs(request.user)

    filesystems.append({
      'file_system': fs,
      'user_home_directory': user_home_dir,
    })

  return JsonResponse(filesystems, safe=False)


@api_error_handler
def upload_file(request):
  # Read request body first to prevent RawPostDataException later on which occurs when trying to access body after it has already been read
  body_data_bytes = string_io(request.body)

  uploaded_file = request.FILES['file']
  dest_path = request.POST.get('destination_path')

  # Check if the file type is restricted
  _, file_type = os.path.splitext(uploaded_file.name)
  if RESTRICT_FILE_EXTENSIONS.get() and file_type.lower() in [ext.lower() for ext in RESTRICT_FILE_EXTENSIONS.get()]:
    return HttpResponse(f'File type "{file_type}" is not allowed. Please choose a file with a different type.', status=400)

  # Check if the file size exceeds the maximum allowed size
  max_size = MAX_FILE_SIZE_UPLOAD_LIMIT.get()
  if max_size >= 0 and uploaded_file.size >= max_size:
    return HttpResponse(f'File exceeds maximum allowed size of {max_size} bytes. Please upload a smaller file.', status=413)

  # Check if the destination path is a directory and the file name contains a path separator
  # This prevents directory traversal attacks
  if request.fs.isdir(dest_path) and posixpath.sep in uploaded_file.name:
    return HttpResponse(f'Invalid filename. Path separators are not allowed.', status=400)

  # Check if the file already exists at the destination path
  filepath = request.fs.join(dest_path, uploaded_file.name)
  if request.fs.exists(filepath):
    return HttpResponse(f'The file path {filepath} already exists.', status=409)

  # Check if the destination path already exists or not
  if not request.fs.exists(dest_path):
    return HttpResponse(f'The destination path {dest_path} does not exist.', status=404)

  try:
    request.fs.upload_v1(request.META, input_data=body_data_bytes, destination=dest_path, username=request.user.username)
  except Exception as ex:
    return HttpResponse(f'Upload to {filepath} failed: {str(ex)}', status=500)

  response = {
    'uploaded_file_stats': _massage_stats(request, stat_absolute_path(filepath, request.fs.stats(filepath))),
  }

  return JsonResponse(response)

def _massage_stats(request, stats):
  """
  Massage a stats record as returned by the filesystem implementation
  into the format that the views would like it in.
  """
  stats_dict = stats.to_json_dict()
  normalized_path = request.fs.normpath(stats_dict.get('path'))

  stats_dict.update(
    {
      'path': normalized_path,
      'type': filetype(stats.mode),
      'rwx': rwx(stats.mode, stats.aclBit),
    }
  )

  return stats_dict
