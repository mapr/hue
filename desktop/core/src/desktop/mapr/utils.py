def get_hive_mechanism():
  """
  This method converts `mechanism` property value from `hue.ini` (''/GSSAPI/MAPR-SECURITY/LDAP)
  to `hive.server2.authentication` property value used in `hive-site.xml` (KERBEROS/NONE/NOSASL/MAPRSASL/LDAP/PAM/CUSTOM)
  """
  from beeswax import conf as beeswax_conf

  mechanism = str(beeswax_conf.MECHANISM.get()).upper()

  HUE_INI_TO_HS2_MECHANISM = {
    '': 'NONE',
    'GSSAPI': 'KERBEROS',
    'MAPR-SECURITY': 'MAPRSASL',
  }

  return HUE_INI_TO_HS2_MECHANISM.get(mechanism, mechanism)

def get_libzookeeper_sasl_options():
  from libzookeeper import conf as libzookeeper_conf
  sasl_options = None

  _mechanism = libzookeeper_conf.MECHANISM.get()
  _principal = libzookeeper_conf.PRINCIPAL_NAME.get()

  if _mechanism == "MAPR-SECURITY":
    sasl_options = {"mechanism": "MAPRSASL"}
  elif _mechanism == "GSSAPI":
    sasl_options = {
      "mechanism": "GSSAPI",
      "principal": _principal,
    }

  return sasl_options
