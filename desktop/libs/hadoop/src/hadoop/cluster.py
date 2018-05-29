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

from django.utils.functional import wraps

from hadoop import conf
from hadoop.fs import webhdfs, LocalSubFileSystem

from desktop.conf import DEFAULT_USER
from desktop.lib.paths import get_build_dir
import subprocess
import threading
from urlparse import urlparse


LOG = logging.getLogger(__name__)


FS_CACHE = None
FS_DEFAULT_NAME = 'default'
MR_CACHE = None # MR now means YARN
MR_NAME_CACHE = 'default'
DEFAULT_USER = DEFAULT_USER.get()



def rm_ha(funct):
  """
  Support RM HA by trying other RM API.
  """
  def decorate(api, *args, **kwargs):
    try:
      return funct(api, *args, **kwargs)
    except Exception, ex:
      ex_message = str(ex)
      if 'Connection refused' in ex_message or 'Connection aborted' in ex_message or 'standby RM' in ex_message:
        LOG.info('Resource Manager not available, trying another RM: %s.' % ex)
        rm_ha = get_next_ha_yarncluster(current_user=api.user)
        if rm_ha is not None:
          if rm_ha[1].url == api.resource_manager_api.url:
            raise ex
          LOG.info('Retrying with Resource Manager: %s.' % rm_ha[1].url)
          config, api.resource_manager_api = rm_ha
          return funct(api, *args, **kwargs)
      raise ex
  return wraps(funct)(decorate)


def get_hdfs(identifier="default"):
  global FS_CACHE
  get_all_hdfs()
  return FS_CACHE[identifier]


def get_defaultfs():
  fs = get_hdfs()

  if fs.logical_name:
    return fs.logical_name
  else:
    return fs.fs_defaultfs


def get_all_hdfs():
  global FS_CACHE
  if FS_CACHE is not None:
    return FS_CACHE

  FS_CACHE = {}
  for identifier in conf.HDFS_CLUSTERS.keys():
    FS_CACHE[identifier] = _make_filesystem(identifier)
  return FS_CACHE


def get_default_yarncluster():
  """
  Get the default RM (not necessarily HA).
  """
  global MR_NAME_CACHE

  try:
    return conf.YARN_CLUSTERS[MR_NAME_CACHE]
  except KeyError:
    return get_yarn()


def get_default_fscluster_config():
  """
  Get the default FS config.
  """
  return conf.HDFS_CLUSTERS[FS_DEFAULT_NAME]


def get_yarn():
  global MR_NAME_CACHE
  if MR_NAME_CACHE in conf.YARN_CLUSTERS and conf.YARN_CLUSTERS[MR_NAME_CACHE].SUBMIT_TO.get():
    return conf.YARN_CLUSTERS[MR_NAME_CACHE]

  for name in conf.YARN_CLUSTERS.keys():
    yarn = conf.YARN_CLUSTERS[name]
    if yarn.SUBMIT_TO.get():
      return yarn


def get_next_ha_yarncluster(current_user=None):
  config_and_rm = get_yarncluster_from_maprcli(current_user)
  if (config_and_rm == None):
    config_and_rm = get_yarncluster_from_config(current_user)

  return config_and_rm


def call_maprcli_urls(servicename):
  try:
    maprcli_popen = subprocess.Popen(["maprcli", "urls", "-name", servicename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()

    if maprcli_stdout.startswith("ERROR"):
      raise Exception(maprcli_stdout)

    url = maprcli_stdout.split("\n")[1]
    url = urlparse(url).scheme + "://" + urlparse(url).netloc.strip()
    LOG.info('%s found: %s.' % (servicename, url))

    return url
  except Exception, ex:
    LOG.info('Error when execute "maprcli urls -name %s": %s' % (servicename, ex))
    return None

YARNCLUSTER_MAPRCLI_CACHED = None
YARNCLUSTER_MAPRCLI_LOCK = threading.Lock()

def get_yarncluster_from_maprcli(current_user=None):
  from hadoop.yarn import mapreduce_api
  from hadoop.yarn import resource_manager_api
  from hadoop.yarn import history_server_api
  from hadoop.yarn import spark_history_server_api
  from hadoop.yarn.resource_manager_api import ResourceManagerApi
  global MR_NAME_CACHE

  global YARNCLUSTER_MAPRCLI_CACHED
  global YARNCLUSTER_MAPRCLI_LOCK

  YARNCLUSTER_MAPRCLI_LOCK.acquire()
  try:
    if YARNCLUSTER_MAPRCLI_CACHED:
      LOG.info('get_yarncluster_from_maprcli(): Trying to use cached RM')
      try:
        (config, rm) = YARNCLUSTER_MAPRCLI_CACHED
        cluster_info = rm.cluster()
        if cluster_info['clusterInfo']['haState'] != 'ACTIVE':
          raise Exception('Cached RM from maprcli is not RUNNING')
      except Exception, ex:
        LOG.info('get_yarncluster_from_maprcli(): Cached RM from maprcli is in bad state, skipping it: %s' % (ex,))
        YARNCLUSTER_MAPRCLI_CACHED = None

    if not YARNCLUSTER_MAPRCLI_CACHED:
      LOG.info('get_yarncluster_from_maprcli(): Run maprcli to update config with right RM and HS')
      name = next((name for name in conf.YARN_CLUSTERS.keys() if conf.YARN_CLUSTERS[name].SUBMIT_TO.get()), None)
      if not name:
        return None

      config = conf.YARN_CLUSTERS[name]
      MR_NAME_CACHE = name

      resourcemanager_url = call_maprcli_urls('resourcemanager')
      historyserver_url = call_maprcli_urls('historyserver')

      if not resourcemanager_url or not historyserver_url:
        return None

      config.HISTORY_SERVER_API_URL.config.default_value = historyserver_url
      config.RESOURCE_MANAGER_API_URL.config.default_value = resourcemanager_url
      config.PROXY_API_URL.config.default_value = resourcemanager_url

      spark_historyserver_url = call_maprcli_urls('spark-historyserver')

      if spark_historyserver_url:
        config.SPARK_HISTORY_SERVER_URL.config.default_value = spark_historyserver_url

    rm = ResourceManagerApi(config.RESOURCE_MANAGER_API_URL.get(), config.SECURITY_ENABLED.get(), config.SSL_CERT_CA_VERIFY.get(), config.MECHANISM.get())
    if current_user is None:
      rm.setuser(DEFAULT_USER)
    else:
      rm.setuser(current_user)

    YARNCLUSTER_MAPRCLI_CACHED = (config, rm)
    resource_manager_api.API_CACHE = None  # Reset cache
    mapreduce_api.API_CACHE = None
    history_server_api.API_CACHE = None
    spark_history_server_api.API_CACHE = None

    return YARNCLUSTER_MAPRCLI_CACHED
  finally:
    YARNCLUSTER_MAPRCLI_LOCK.release()


def get_yarncluster_from_config(current_user=None):
  """
  Return the next available YARN RM instance and cache its name.
  """
  from hadoop.yarn.resource_manager_api import ResourceManagerApi
  global MR_NAME_CACHE

  has_ha = sum([conf.YARN_CLUSTERS[name].SUBMIT_TO.get() for name in conf.YARN_CLUSTERS.keys()]) >= 2

  for name in conf.YARN_CLUSTERS.keys():
    config = conf.YARN_CLUSTERS[name]
    if config.SUBMIT_TO.get():
      rm = ResourceManagerApi(config.RESOURCE_MANAGER_API_URL.get(), config.SECURITY_ENABLED.get(), config.SSL_CERT_CA_VERIFY.get(), config.MECHANISM.get())
      if current_user is None:
        rm.setuser(DEFAULT_USER)
      else:
        rm.setuser(current_user)
      if has_ha:
        try:
          cluster_info = rm.cluster()
          if cluster_info['clusterInfo']['haState'] == 'ACTIVE':
            if name != MR_NAME_CACHE:
              LOG.info('RM %s has failed back to %s server' % (MR_NAME_CACHE, name))
              rm.from_failover = True
            MR_NAME_CACHE = name
            LOG.warn('Picking RM HA: %s' % name)
            return (config, rm)
          else:
            LOG.info('RM %s is not RUNNING, skipping it: %s' % (name, cluster_info))
        except Exception, ex:
          LOG.exception('RM %s is not available, skipping it: %s' % (name, ex))
      else:
        return (config, rm)
  return None


def get_cluster_for_job_submission():
  """
  Check the 'submit_to' for each MR/Yarn cluster, and return the
  config section of first one that enables submission.

  Support MR1/MR2 HA.
  """
  yarn = get_next_ha_yarncluster()
  if yarn:
    return yarn

  return None


def get_cluster_conf_for_job_submission():
  cluster = get_cluster_for_job_submission()

  if cluster:
    config, rm = cluster
    return config
  else:
    return None


def get_cluster_addr_for_job_submission():
  """
  Check the 'submit_to' for each MR/Yarn cluster, and return the logical name or host:port of first one that enables submission.
  """
  if is_yarn():
    if get_yarn().LOGICAL_NAME.get():
      return get_yarn().LOGICAL_NAME.get()

  conf = get_cluster_conf_for_job_submission()
  if conf is None:
    return None

  return "%s:%s" % (conf.HOST.get(), conf.PORT.get())


def is_yarn():
  return get_yarn() is not None


def clear_caches():
  """
  Clears cluster's internal caches.  Returns
  something that can be given back to restore_caches.
  """
  global FS_CACHE
  old = FS_CACHE
  FS_CACHE = None
  return old


def restore_caches(old):
  """
  Restores caches from the result of a previous clear_caches call.
  """
  global FS_CACHE
  FS_CACHE = old


def _make_filesystem(identifier):
  choice = os.getenv("FB_FS")

  if choice == "testing":
    path = os.path.join(get_build_dir(), "fs")
    if not os.path.isdir(path):
      LOG.warning(("Could not find fs directory: %s. Perhaps you need to run manage.py filebrowser_test_setup?") % path)
    return LocalSubFileSystem(path)
  else:
    cluster_conf = conf.HDFS_CLUSTERS[identifier]
    return webhdfs.WebHdfs.from_config(cluster_conf)
