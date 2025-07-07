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
import os

from filebrowser.conf import ALLOW_FILE_EXTENSIONS, RESTRICT_FILE_EXTENSIONS

LOG = logging.getLogger()

def is_file_upload_allowed(file_name):
  """
  Check if a file upload is allowed based on file extension restrictions.

  Args:
    file_name: The name of the file being uploaded

  Returns:
    tuple: (is_allowed, error_message)
      - is_allowed: Boolean indicating if the file upload is allowed
      - error_message: String with error message if not allowed, None otherwise
  """
  if not file_name:
    return True, None

  _, file_type = os.path.splitext(file_name)
  if file_type:
    file_type = file_type.lower()

  # Check allow list first - if set, only these extensions are allowed
  allow_list = ALLOW_FILE_EXTENSIONS.get()
  if allow_list:
    # Normalize extensions to lowercase with dots
    normalized_allow_list = [ext.lower() if ext.startswith(".") else f".{ext.lower()}" for ext in allow_list]
    if file_type not in normalized_allow_list:
      return False, f'File type "{file_type}" is not permitted. Modify file extension settings to allow this type.'

  # Check restrict list - if set, these extensions are not allowed
  restrict_list = RESTRICT_FILE_EXTENSIONS.get()
  if restrict_list:
    # Normalize extensions to lowercase with dots
    normalized_restrict_list = [ext.lower() if ext.startswith(".") else f".{ext.lower()}" for ext in restrict_list]
    if file_type in normalized_restrict_list:
      return False, f'File type "{file_type}" is restricted. Update file extension restrictions to allow this type.'

  return True, None
