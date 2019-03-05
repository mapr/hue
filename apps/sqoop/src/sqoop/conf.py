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
import os

from django.utils.translation import ugettext_lazy as _t

from desktop.conf import default_ssl_validate
from desktop.lib.conf import Config, coerce_bool

from sqoop.settings import NICE_NAME

LOG = logging.getLogger(__name__)

SERVER_URL = Config(
  key="server_url",
  default='http://localhost:12000/sqoop',
  help=_t("The sqoop server URL."))

SQOOP_CONF_DIR = Config(
  key="sqoop_conf_dir",
  default='/etc/sqoop2/conf',
  help=_t("Path to Sqoop2 configuration directory."))

SSL_CERT_CA_VERIFY = Config(
  key="ssl_cert_ca_verify",
  help=_t("Choose whether Hue should validate certificates received from the server."),
  dynamic_default=default_ssl_validate,
  type=coerce_bool
)

SECURITY_ENABLED=Config("security_enabled", help="Is running with Kerberos or MapR-securtity authentication",
                        default=False, type=coerce_bool)

MECHANISM = Config("mechanism", help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY",
                   default='none', type=str)

IS_ENABLED = Config(
    key="is_enabled",
    help=_t("If the Sqoop2 app is enabled. Sqoop2 project is deprecated. Sqoop1 is recommended."),
    type=coerce_bool,
    default=False
)

def config_validator(user):
  res = []

  try:
    from sqoop import client
    c = client.SqoopClient(url=SERVER_URL.get(), username=user.username, security_enabled=SECURITY_ENABLED.get(), mechanism=MECHANISM.get())
    c.get_connectors()
  except Exception, ex:
    LOG.exception('Sqoop2 API is not accessible: %s' % ex)
    res.append((NICE_NAME, _('Sqoop2 API is not accessible')))

  return res
