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
import posixpath
import requests
import threading

from desktop import conf as desktop_conf
from desktop.lib.exceptions_renderable import PopupException
from desktop.lib.rest.http_client import HttpClient
from desktop.lib.rest.resource import Resource
from hadoop import cluster


LOG = logging.getLogger(__name__)

_API_VERSION = 'v1'
_JSON_CONTENT_TYPE = 'application/json'


# copied from desktop.lib.res.http_client.get_request_session
CACHE_SESSION_SPARK_HS = None
CACHE_SESSION_LOCK_SPARK_HS = threading.Lock()

def get_request_session_for_spark_hs():
  global CACHE_SESSION_SPARK_HS
  if CACHE_SESSION_SPARK_HS is None:
    CACHE_SESSION_LOCK_SPARK_HS.acquire()
    try:
      if CACHE_SESSION_SPARK_HS is None:
        CACHE_SESSION_SPARK_HS = requests.Session()
        CACHE_SESSION_SPARK_HS.mount('http://', requests.adapters.HTTPAdapter(pool_connections=desktop_conf.CHERRYPY_SERVER_THREADS.get(), pool_maxsize=desktop_conf.CHERRYPY_SERVER_THREADS.get()))
        CACHE_SESSION_SPARK_HS.mount('https://', requests.adapters.HTTPAdapter(pool_connections=desktop_conf.CHERRYPY_SERVER_THREADS.get(), pool_maxsize=desktop_conf.CHERRYPY_SERVER_THREADS.get()))
    finally:
      CACHE_SESSION_LOCK_SPARK_HS.release()
  return CACHE_SESSION_SPARK_HS


API_CACHE = None
API_CACHE_LOCK = threading.Lock()


def get_history_server_api():
  # TODO: Spark History Server does not yet support setuser, implement when it does
  global API_CACHE

  if API_CACHE is None:
    API_CACHE_LOCK.acquire()
    try:
      if API_CACHE is None:
        yarn_cluster = cluster.get_cluster_conf_for_job_submission()
        if yarn_cluster is None:
          raise PopupException(_('No Spark History Server is available.'))
        API_CACHE = SparkHistoryServerApi(yarn_cluster.SPARK_HISTORY_SERVER_URL.get(), yarn_cluster.SECURITY_ENABLED.get(), yarn_cluster.SSL_CERT_CA_VERIFY.get(), yarn_cluster.MECHANISM.get())
    finally:
      API_CACHE_LOCK.release()

  return API_CACHE


class SparkHistoryServerApi(object):

  def __init__(self, spark_hs_url, security_enabled=False, ssl_cert_ca_verify=False, mechanism=None):
    self._ui_url = spark_hs_url
    self._url = posixpath.join(spark_hs_url, 'api/%s/' % _API_VERSION)
    self._client = HttpClient(self._url, logger=LOG)
    self._client._session = get_request_session_for_spark_hs()
    self._root = Resource(self._client)
    self._security_enabled = security_enabled

    if self._security_enabled and mechanism == 'GSSAPI':
      self._client.set_kerberos_auth()

    self._client.set_verify(ssl_cert_ca_verify)

  def __str__(self):
    return "Spark History Server API at %s" % (self._url,)

  @property
  def url(self):
    return self._url

  @property
  def ui_url(self):
    return self._ui_url

  @property
  def headers(self):
    return {'Accept': _JSON_CONTENT_TYPE}

  def applications(self):
    return self._root.get('applications', headers=self.headers)

  def application(self, app_id):
    return self._root.get('applications/%(app_id)s' % {'app_id': app_id}, headers=self.headers)

  def jobs(self, app_id, attempt_id):
    return self._root.get('applications/%(app_id)s/%(attempt_id)s/jobs' % {'app_id': app_id, 'attempt_id': attempt_id}, headers=self.headers)

  def stages(self, app_id, attempt_id):
    return self._root.get('applications/%(app_id)s/%(attempt_id)s/stages' % {'app_id': app_id, 'attempt_id': attempt_id}, headers=self.headers)

  def executors(self, app_id, attempt_id):
    return self._root.get('applications/%(app_id)s/%(attempt_id)s/executors' % {'app_id': app_id, 'attempt_id': attempt_id}, headers=self.headers)

  # TODO: stage attempts, task summaries, task list, storage, download logs
  # http://spark.apache.org/docs/latest/monitoring.html#rest-api
