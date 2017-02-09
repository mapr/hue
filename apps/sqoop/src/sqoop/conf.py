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

from django.utils.translation import ugettext_lazy as _t

from desktop.lib.conf import Config, coerce_bool
from sqoop.settings import NICE_NAME


SERVER_URL = Config(
  key="server_url",
  default='http://localhost:12000/sqoop',
  help=_t("The sqoop server URL."))

SECURITY_ENABLED=Config("security_enabled", help="Is running with Kerberos or MapR-securtity authentication",
                              default=False, type=coerce_bool)

MECHANISM = Config("mechanism",
                   help="Security mechanism of authentication none/GSSAPI/MAPR-SECURITY",
                   default='none',
                   type=str)

SQOOP_CONF_DIR = Config(
  key="sqoop_conf_dir",
  default='/opt/mapr/sqoop/sqoop-2.0.0/conf',
  help=_t("Path to Sqoop2 configuration directory."))
