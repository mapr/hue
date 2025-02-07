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

from django.utils.translation import ugettext_lazy as _t
from desktop.lib.conf import Config, UnspecifiedConfigSection, ConfigSection, coerce_bool
from desktop.conf import default_ssl_validate
import fnmatch
import logging
import os

DEFAULT_NN_HTTP_PORT = 50070

LOG = logging.getLogger(__name__)

def find_file_recursive(desired_glob, root):
  def f():
    for dirpath, dirnames, filenames in os.walk(root):
      matches = fnmatch.filter(filenames, desired_glob)
      if matches:
        if len(matches) != 1:
          logging.warning("Found multiple jars matching %s: %s" %
                          (desired_glob, matches))
        return os.path.join(dirpath, matches[0])

    logging.error("Trouble finding jars matching %s" % (desired_glob,))
    return None

  f.__doc__ = "Finds %s/%s" % (root, desired_glob)
  return f


UPLOAD_CHUNK_SIZE = Config(
  key="upload_chunk_size",
  help="Size, in bytes, of the 'chunks' Django should store into memory and feed into the handler. Default is 64MB.",
  type=int,
  default=1024 * 1024 * 64)


HDFS_CLUSTERS = UnspecifiedConfigSection(
  "hdfs_clusters",
  help="One entry for each HDFS cluster",
  each=ConfigSection(
    help="Information about a single HDFS cluster",
    members=dict(
      FS_DEFAULTFS=Config("fs_defaultfs", help="The equivalent of fs.defaultFS (aka fs.default.name)",
                          default="hdfs://localhost:8020"),
      LOGICAL_NAME = Config("logical_name", default="",
                            type=str, help=_t('NameNode logical name.')),
      WEBHDFS_URL=Config("webhdfs_url",
                         help="The URL to WebHDFS/HttpFS service. Defaults to " +
                         "the WebHDFS URL on the NameNode.",
                         type=str, default="http://localhost:50070/webhdfs/v1"),
      NN_KERBEROS_PRINCIPAL=Config("nn_kerberos_principal", help="Kerberos principal for NameNode", # Unused
                                   default="hdfs", type=str),
      DN_KERBEROS_PRINCIPAL=Config("dn_kerberos_principal", help="Kerberos principal for DataNode", # Unused
                                   default="hdfs", type=str),
      SECURITY_ENABLED=Config("security_enabled", help="Is running with Kerberos or MapR-securtity authentication",
                              default=False, type=coerce_bool),
      SSL_CERT_CA_VERIFY=Config("ssl_cert_ca_verify",
                  help="In secure mode (HTTPS), if SSL certificates from YARN Rest APIs have to be verified against certificate authority",
                  dynamic_default=default_ssl_validate,
                  type=coerce_bool),
      MECHANISM=Config("mechanism",
                  default='none',
                  help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY",
                  type=str),
      TEMP_DIR=Config("temp_dir", help="HDFS directory for temporary files",
                      default='/tmp', type=str),
      FILE_SIZE=Config("file_size",
                       help="File size restriction for viewing file", default=1.0, type=float),
      HADOOP_CONF_DIR = Config(
        key="hadoop_conf_dir",
        default=os.environ.get("HADOOP_CONF_DIR", "/etc/hadoop/conf"),
        help=("Directory of the Hadoop configuration) Defaults to the environment variable " +
              "HADOOP_CONF_DIR when set, or '/etc/hadoop/conf'."),
      ),
      MUTUAL_SSL_AUTH=Config("mutual_ssl_auth", help="Is running with mutual SSL authentication",
                              default=False, type=coerce_bool),
      SSL_CERT=Config("ssl_cert", help="Certificate for SSL connection",
                         type=str, default="/opt/mapr/hue/hue-3.9.0/cert.pem"),
      SSL_KEY=Config("ssl_key", help="Private key for SSL connection",
                         type=str, default="/opt/mapr/hue/hue-3.9.0/hue_private_keystore.pem")
    )
  )
)

MR_CLUSTERS = UnspecifiedConfigSection(
  "mapred_clusters",
  help="One entry for each MapReduce cluster",
  each=ConfigSection(
    help="Information about a single MapReduce cluster",
    members=dict(
      HOST=Config("jobtracker_host", help="Host/IP for JobTracker"),
      PORT=Config("jobtracker_port",
                  default=9001,
                  help="Service port for the JobTracker",
                  type=int),
      LOGICAL_NAME=Config('logical_name',
                          default="",
                          type=str,
                          help=_t('JobTracker logical name.')),
      JT_THRIFT_PORT=Config("thrift_port", help="Thrift port for JobTracker", default=9290,
                            type=int),
      JT_KERBEROS_PRINCIPAL=Config("jt_kerberos_principal", help="Kerberos principal for JobTracker",
                                   default="mapr", type=str),
      SECURITY_ENABLED=Config("security_enabled", help="Is running with Kerberos authentication",
                              default=False, type=coerce_bool),
      SUBMIT_TO=Config('submit_to', help="Whether Hue should use this cluster to run jobs",
                       default=True, type=coerce_bool), # True here for backward compatibility
      MECHANISM=Config("mechanism", help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY",
                       default='none', type=str)
    )
  )
)

YARN_CLUSTERS = UnspecifiedConfigSection(
  "yarn_clusters",
  help="One entry for each Yarn cluster",
  each=ConfigSection(
    help="Information about a single Yarn cluster",
    members=dict(
      HOST=Config("resourcemanager_host",
                  default='localhost',
                  help="Host/IP for the ResourceManager"),
      PORT=Config("resourcemanager_port",
                  default=8032,
                  type=int,
                  help="Service port for the ResourceManager"),
      LOGICAL_NAME=Config('logical_name',
                          default="",
                          type=str,
                          help=_t('Resource Manager logical name.')),
      SECURITY_ENABLED=Config("security_enabled", help="Is running with Kerberos authentication",
                              default=False, type=coerce_bool),
      SUBMIT_TO=Config('submit_to', help="Whether Hue should use this cluster to run jobs",
                       default=False, type=coerce_bool), # False here for backward compatibility
      IS_YARN=Config("is_yarn", help="Attribute set only on YARN clusters and not MR1 ones.",
                     default=True, type=coerce_bool),
      RESOURCE_MANAGER_API_URL=Config("resourcemanager_api_url",
                  default='http://localhost:8088',
                  help="URL of the ResourceManager API"),
      PROXY_API_URL=Config("proxy_api_url",
                  default='http://localhost:8088',
                  help="URL of the ProxyServer API"),
      HISTORY_SERVER_API_URL=Config("history_server_api_url",
                  default='http://localhost:19888',
                  help="URL of the HistoryServer API"),
      SSL_CERT_CA_VERIFY=Config("ssl_cert_ca_verify",
                  help="In secure mode (HTTPS), if SSL certificates from YARN Rest APIs have to be verified against certificate authority",
                  dynamic_default=default_ssl_validate,
                  type=coerce_bool),
      MECHANISM=Config("mechanism",
                  default='none',
                  help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY",
                  type=str)
    )
  )
)


def config_validator(user):
  """
  config_validator() -> [ (config_variable, error_message) ]

  Called by core check_config() view.
  """
  from hadoop.fs import webhdfs
  from hadoop import job_tracker

  res = []
  submit_to = []

  # HDFS_CLUSTERS
  has_default = False
  for name in HDFS_CLUSTERS.keys():
    cluster = HDFS_CLUSTERS[name]
    res.extend(webhdfs.test_fs_configuration(cluster))
    if name == 'default':
      has_default = True
  if not has_default:
    res.append(("hadoop.hdfs_clusters", "You should have an HDFS called 'default'."))

  # MR_CLUSTERS
  mr_down = []
  for name in MR_CLUSTERS.keys():
    cluster = MR_CLUSTERS[name]
    if cluster.SUBMIT_TO.get():
      mr_down.extend(job_tracker.test_jt_configuration(cluster))
      submit_to.append('mapred_clusters.' + name)
  # If HA still failing
  if mr_down and len(mr_down) == len(MR_CLUSTERS.keys()):
    res.extend(mr_down)

  # YARN_CLUSTERS
  for name in YARN_CLUSTERS.keys():
    cluster = YARN_CLUSTERS[name]
    if cluster.SUBMIT_TO.get():
      res.extend(test_yarn_configurations())
      submit_to.append('yarn_clusters.' + name)

  if not submit_to:
    res.append(("hadoop", "Please designate one of the MapReduce or "
                "Yarn clusters with `submit_to=true' in order to run jobs."))

  return res


def test_yarn_configurations():
  # Single cluster for now
  from hadoop.yarn.resource_manager_api import get_resource_manager

  result = []

  try:
    url = ''
    api = get_resource_manager()
    url = api._url
    api.apps()
  except Exception, e:
    msg = 'Failed to contact Resource Manager at %s: %s' % (url, e)
    LOG.exception(msg)
    result.append(('Resource Manager', msg))

  return result
