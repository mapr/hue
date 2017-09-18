import logging
import re
import os


LOG = logging.getLogger(__name__)


from desktop.lib.paths import get_run_root


MAPR_HOME = os.environ.get('MAPR_HOME', '/opt/mapr')
MAPR_CLUSTERS_CONF_PATH = os.path.join(MAPR_HOME, 'conf',  'mapr-clusters.conf')


MAPR_SECURITY = "MAPR-SECURITY"
SECURE = "secure"
SECURITY_ENABLED = 'security_enabled'
MECHANISM = 'mechanism'
SSL_CACERTS = 'ssl_cacerts'
SSL_CERTIFICATE = 'ssl_certificate'
SSL_PRIVATE_KEY = 'ssl_private_key'


SSL_CACERTS_PATH = os.path.join(get_run_root(), 'keys', 'cert.pem')
SSL_CERTIFICATE_PATH = os.path.join(get_run_root(), 'keys', 'cert.pem')
SSL_PRIVATE_KEY_PATH = os.path.join(get_run_root(), 'keys', 'hue_private_keystore.pem')


templates = {
  MECHANISM: 'none',
  SECURITY_ENABLED: 'false',
  SSL_CACERTS: None,
  SSL_CERTIFICATE: None,
  SSL_PRIVATE_KEY: None,
}


def read_values_from_mapr_clusters_conf():
  if not os.path.exists(MAPR_CLUSTERS_CONF_PATH):
    return

  mapr_clusters_conf = open(MAPR_CLUSTERS_CONF_PATH, "r").read()
  cluster_props = dict(re.findall(r'(\S+)=(".*?"|\S+)', mapr_clusters_conf))

  templates[SECURITY_ENABLED] = cluster_props[SECURE] if SECURE in cluster_props else "false"

  if templates[SECURITY_ENABLED] == "true":
    templates[MECHANISM] = MAPR_SECURITY

    if os.path.exists(SSL_CERTIFICATE_PATH) and os.path.exists(SSL_PRIVATE_KEY_PATH) and os.path.exists(SSL_CACERTS_PATH):
      templates[SSL_CACERTS] = SSL_CACERTS_PATH
      templates[SSL_CERTIFICATE] = SSL_CERTIFICATE_PATH
      templates[SSL_PRIVATE_KEY] = SSL_PRIVATE_KEY_PATH
    else:
      msg = ("Cluster configured with security, "
             "but files %s, %s and %s do not exist, "
             "so Hue will not run over HTTPS. "
             "Falling back to HTTP."
            ) % (SSL_CACERTS, SSL_CERTIFICATE_PATH, SSL_PRIVATE_KEY_PATH)
      LOG.exception(msg)


templateRegEx = re.compile(r'^\${(.+?)}')

def change_config(config):
  for key in config:
    if isinstance(config[key], dict):
      change_config(config[key])
    elif type(config[key]) == str:
      match = templateRegEx.search(config[key])
      if (match != None) and (match.group(1) in templates):
        config[key] = templates[match.group(1)]

  return config

def fill_templates(config):
  read_values_from_mapr_clusters_conf()
  change_config(config)

  return config

