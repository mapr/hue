#!/usr/bin/env python
# -- coding: utf-8 --
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

import binascii
import logging
import re

from github import Github
from github.GithubException import UnknownObjectException


LOG = logging.getLogger(__name__)


class GithubImportException(Exception):
  pass


class GithubImporter(object):

  REPO_URL_RE = re.compile("http[s]?://(www.)?github.com/([a-z0-9](?:-?[a-z0-9]){0,38}).([\w\.@\:\-~]+)[/]?")

  def __init__(self, user=None, token=None):
    if user and token:
      self._client = Github(user, token)
    else:
      self._client = Github()  # anonymous Github API access, public repos only


  @classmethod
  def get_username_and_repo(cls, url):
    """
    Given a base URL to a Github repository, return a tuple of the username and repo name
    :param url: base URL to repo
    :return: tuple of username and repo
    """
    match = cls.REPO_URL_RE.search(url)
    if match:
      return match.group(2), match.group(3)
    else:
      raise ValueError('Github URL is not formatted correctly: %s' % url)


  def get_file_contents(self, username, repo, filepath):
    content = ''
    filepath = filepath.strip('/')

    try:
      user = self._client.get_user(username)
      repo = user.get_repo(repo)
      file = repo.get_file_contents(filepath)
      content = file.content.decode('base64')
    except binascii.Error:
      raise GithubImportException('Failed to decode file contents, check if file content is properly base64-encoded.')
    except UnknownObjectException:
      raise GithubImportException('Could not find Github object, check username, repo and filepath or permissions.')
    except Exception, e:
      raise GithubImportException('Failed to get file content from Github: %s' % str(e))

    return content
