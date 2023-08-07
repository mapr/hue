import logging
import os
import threading

from desktop.lib import conf as conf_lib


LOG = logging.getLogger(__name__)

MAPR_USER = os.environ.get('MAPR_USER', 'mapr')


def run_once(func):
  def wrapper(*args, **kwargs):
    if not wrapper.has_run:
      wrapper.has_run = True
      return func(*args, **kwargs)
  wrapper.has_run = False
  return wrapper

def synchronized(func):
  lock = threading.Lock()
  def wrapper(*args, **kwargs):
    with lock:
      return func(*args, **kwargs)
  return wrapper

@synchronized
@run_once
def patch_desktop_conf():
  #
  # desktop.conf
  #
  from desktop import conf as desktop_conf

  # MySQL connector fix
  desktop_conf_coerce_database = desktop_conf.coerce_database

  def patched_coerce_database(database):
    if database == 'mysql':
      return 'mysql.connector.django'
    else:
      return desktop_conf_coerce_database(database)

  desktop_conf.DATABASE.members['ENGINE'].type = patched_coerce_database

  # multiple desktop.auth.pam_service
  desktop_conf.AUTH.members['PAM_SERVICE'].default_value = 'sudo sshd login'

  # Default user
  desktop_conf.LDAP_USERNAME.devault_value = MAPR_USER
  desktop_conf.SERVER_USER.devault_value = MAPR_USER
  desktop_conf.SERVER_GROUP.devault_value = MAPR_USER
  desktop_conf.DEFAULT_USER.devault_value = MAPR_USER
  desktop_conf.DEFAULT_HDFS_SUPERUSER.devault_value = MAPR_USER
  desktop_conf.DEFAULT_HDFS_SUPERUSER.help = "This should be the hadoop cluster admin, defaults to owner of maprfs:///var"

  # desktop.kerberos.ccache_path
  desktop_conf.KERBEROS.members['CCACHE_PATH'].default_value = '/tmp/hue_krb5_ccache'

  # desktop.ssl_cipher_list
  desktop_conf.SSL_CIPHER_LIST.default_value = ':'.join([
      desktop_conf.SSL_CIPHER_LIST.default_value,
      '!SSLv2',
      '!SSLv3',
      '!TLSv1',
      '!TLSv1.1',
      'TLSv1.2',
    ])

  # desktop.auth.ensure_home_directory
  desktop_conf.AUTH.update_members({
    'ENSURE_HOME_DIRECTORY': conf_lib.Config(
      key='ensure_home_directory',
      help="Ensure that users home directory exists in DFS on login.",
      type=conf_lib.coerce_bool,
      default=True,
    ),
  })

  desktop_conf.SECURE_CONTENT_SECURITY_POLICY.default_value = (
    "script-src 'self' 'unsafe-inline' 'unsafe-eval' *.google-analytics.com *.doubleclick.net data:;"
    "img-src 'self' *.google-analytics.com *.doubleclick.net http://*.tile.osm.org *.tile.osm.org *.gstatic.com data:;"
    "style-src 'self' 'unsafe-inline' fonts.googleapis.com;"
    "connect-src 'self';"
    "frame-src *;"
    "child-src 'self' data: blob: *.vimeo.com;"
    "object-src 'none'"
  )

@synchronized
@run_once
def patch_lib_conf():
  #
  # hadoop.conf
  #
  from hadoop import conf as hadoop_conf

  # hadoop.hdfs_clusters
  hadoop_conf.HDFS_CLUSTERS.each.members['FS_DEFAULTFS'].default_value = 'maprfs:///'
  hadoop_conf.HDFS_CLUSTERS.each.members['WEBHDFS_URL'].default_value = 'http://localhost:14000/webhdfs/v1'
  hadoop_conf.HDFS_CLUSTERS.each.members['SECURITY_ENABLED'].help = "Is running with Kerberos or MapR-Securtity authentication."

  hadoop_conf.HDFS_CLUSTERS.each.update_members({
    'MECHANISM': conf_lib.Config(
      key='mechanism',
      help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY.",
      default='none',
    ),
    'MUTUAL_SSL_AUTH': conf_lib.Config(
      key='mutual_ssl_auth',
      help="Enable mutual SSL authentication",
      type=conf_lib.coerce_bool,
      default=False,
    ),
    'SSL_CERT': conf_lib.Config(
      key='ssl_cert',
      help="Certificate for SSL connection",
      default='keys/cert.pem',
    ),
    'SSL_KEY': conf_lib.Config(
      key='ssl_key',
      help="Private key for SSL connection",
      default='keys/hue_private_keystore.pem',
    ),
  })

  # hadoop.yarn_clusters
  hadoop_conf.YARN_CLUSTERS.each.update_members({
    'MECHANISM': conf_lib.Config(
      key='mechanism',
      help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY.",
      default='none',
    ),
  })

  #
  # librdbms.conf
  #
  from librdbms import conf as librdbms_conf

  librdbms_conf.DATABASES.each.update_members({
    'CONNECTION_TYPE': conf_lib.Config(
      key='connection_type',
      help=("Connection type. This can be:\n"
            "1. direct\n"
            "2. zookeeper\n"),
      default='direct',
    ),
    'DRILLBITS': conf_lib.Config(
      key='drillbits',
      help="Drillbit address for direct connection.",
      type=conf_lib.coerce_string,
      default='localhost:31010',
    ),
    'ZK_QUORUM': conf_lib.Config(
      key='zk_quorum',
      help="ZooKeeper quorum for connection through ZooKeeper.",
      type=conf_lib.coerce_string,
      default='localhost:5181',
    ),
    'ZK_CLUSTER_ID': conf_lib.Config(
      key='zk_cluster_id',
      help="Set ZKClusterID to the name of the Drillbit cluster to use.",
      default='',
    ),
    'MECHANISM': conf_lib.Config(
      key='mechanism',
      help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY.",
      default='none',
    ),
  })

  #
  # notebook.conf
  #
  from notebook import conf as notebook_conf

  notebook_conf.INTERPRETERS.each.members['OPTIONS'].preserve_subs = ['USER', 'PASSWORD']

@synchronized
@run_once
def patch_app_conf():
  #
  # beeswax.conf
  #
  from beeswax import conf as beeswax_conf

  beeswax_conf.MECHANISM = conf_lib.Config(
    key='mechanism',
    help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY.",
    default='none',
  )

  #
  # spark.conf
  #
  from spark import conf as spark_conf

  spark_conf.MECHANISM = conf_lib.Config(
    key='mechanism',
    help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY.",
    default='none',
  )

  #
  # hbase.conf
  #
  from hbase import conf as hbase_conf

  hbase_conf.MECHANISM = conf_lib.Config(
    key='mechanism',
    help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY.",
    default='none',
  )
