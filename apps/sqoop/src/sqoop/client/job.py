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

from desktop.lib.python_util import force_dict_to_strings

from exception import SqoopException
from config import Config


class Job(object):

  SKIP = ('id', 'creation_date', 'creation_user', 'update_date', 'update_user')

  def __init__(self, name, from_link_id, to_link_id, from_connector_id, to_connector_id, from_config_values=None, to_config_values=None, driver_config_values=None, enabled=True, creation_user='hue', creation_date=0, update_user='hue', update_date=0, from_connector_name=None, to_connector_name=None, from_link_name=None, to_link_name=None, **kwargs):
    self.id = kwargs.setdefault('id', -1)
    self.creation_user = creation_user
    self.creation_date = creation_date
    self.update_user = update_user
    self.update_date = update_date
    self.enabled = enabled
    self.name = name
    self.from_link_id = from_link_id
    self.from_link_name = from_link_name
    self.to_link_id = to_link_id
    self.to_link_name = to_link_name
    self.from_connector_id = from_connector_id
    self.from_connector_name = from_connector_name
    self.to_connector_id = to_connector_id
    self.to_connector_name = to_connector_name
    self.from_config_values = from_config_values
    self.to_config_values = to_config_values
    self.driver_config_values = driver_config_values

  @staticmethod
  def from_dict(job_dict):
    job_dict.setdefault('from_config_values', [])
    job_dict['from_config_values'] = [ Config.from_dict(from_config_value_dict) for from_config_value_dict in job_dict['from-config-values'] ]

    job_dict.setdefault('to_config_values', [])
    job_dict['to_config_values'] = [ Config.from_dict(to_config_value_dict) for to_config_value_dict in job_dict['to-config-values'] ]

    job_dict.setdefault('driver_config_values', [])
    job_dict['driver_config_values'] = [ Config.from_dict(driver_config_value_dict) for driver_config_value_dict in job_dict['driver-config-values'] ]

    if not 'from_link_id' in job_dict:
      job_dict['from_link_id'] = job_dict.get('from-link-id', -1)

    if not 'from_link_name' in job_dict:
      job_dict['from_link_name'] = job_dict['from-link-name']

    if not 'to_link_id' in job_dict:
      job_dict['to_link_id'] = job_dict.get('to-link-id', -1)

    if not 'to_link_name' in job_dict:
      job_dict['to_link_name'] = job_dict['to-link-name']

    if not 'from_connector_id' in job_dict:
      job_dict['from_connector_id'] = job_dict.get('from-connector-id', -1)

    if not 'from_connector_name' in job_dict:
      job_dict['from_connector_name'] = job_dict['from-connector-name']

    if not 'to_connector_id' in job_dict:
      job_dict['to_connector_id'] = job_dict.get('to-connector-id', -1)

    if not 'to_connector_name' in job_dict:
      job_dict['to_connector_name'] = job_dict['to-connector-name']

    if not 'creation_user' in job_dict:
      job_dict['creation_user'] = job_dict.setdefault('creation-user', 'hue')

    if not 'creation_date' in job_dict:
      job_dict['creation_date'] = job_dict.setdefault('creation-date', 0)

    if not 'update_user' in job_dict:
      job_dict['update_user'] = job_dict.setdefault('update-user', 'hue')

    if not 'update_date' in job_dict:
      job_dict['update_date'] = job_dict.setdefault('update-date', 0)

    return Job(**force_dict_to_strings(job_dict))

  @staticmethod
  def from_dict_new(job_dict):
    job_dict['from-config-values'] = job_dict['from-config-values']['configs']
    job_dict['to-config-values'] = job_dict['to-config-values']['configs']
    job_dict['driver-config-values'] = job_dict['driver-config-values']['configs']

    return Job.from_dict(job_dict)

  def to_dict(self):
    d = {
      'id': self.id,
      'name': self.name,
      'creation-user': self.creation_user,
      'creation-date': self.creation_date,
      'update-user': self.update_user,
      'update-date': self.update_date,
      'from-link-id': self.from_link_id,
      'from-link-name': self.from_link_name,
      'to-link-id': self.to_link_id,
      'to-link-name': self.to_link_name,
      'from-connector-id': self.from_connector_id,
      'from-connector-name': self.from_connector_name,
      'to-connector-id': self.to_connector_id,
      'to-connector-name': self.to_connector_name,
      'from-config-values': [ from_config_value.to_dict() for from_config_value in self.from_config_values ],
      'to-config-values': [ to_config_value.to_dict() for to_config_value in self.to_config_values ],
      'driver-config-values': [ driver_config_value.to_dict() for driver_config_value in self.driver_config_values ],
      'enabled': self.enabled
    }
    return d

  def to_dict_new(self):
    d = self.to_dict()

    for config in d['from-config-values']:
      config['validators'] = []
      for input in config['inputs']:
        input['validators'] = []
        input['overrides'] = ''
    d['from-config-values'] = {'configs': d['from-config-values'], 'validators': []}

    for config in d['to-config-values']:
      config['validators'] = []
      for input in config['inputs']:
        input['validators'] = []
        input['overrides'] = ''
    d['to-config-values'] = {'configs': d['to-config-values'], 'validators': []}

    for config in d['driver-config-values']:
      config['validators'] = []
      for input in config['inputs']:
        input['validators'] = []
        input['overrides'] = ''
    d['driver-config-values'] = {'configs': d['driver-config-values'], 'validators': []}

    return d

  def update_from_dict(self, job_dict):
    self.update(Job.from_dict(job_dict))

  def update(self, job):
    for key in self.__dict__:
      if key not in Job.SKIP:
        setattr(self, key, getattr(job, key, getattr(self, key)))
