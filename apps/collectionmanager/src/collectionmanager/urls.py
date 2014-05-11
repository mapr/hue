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

from django.conf.urls.defaults import patterns, url

urlpatterns = patterns('collectionmanager.views',
  url(r'^$', 'collections', name='index')
)

urlpatterns += patterns('collectionmanager.api',
  url(r'^api/fields/parse/$', 'parse_fields', name='api_parse_fields'),
  url(r'^api/schema/example/$', 'example_schema', name='api_example_schema'),
  url(r'^api/collections/$', 'collections', name='api_collections'),
  url(r'^api/collections/create/$', 'collections_create', name='api_collections_create'),
  url(r'^api/collections/import/$', 'collections_import', name='api_collections_import'),
  url(r'^api/collections/remove/$', 'collections_remove', name='api_collections_remove'),
  url(r'^api/collections/(?P<collection_or_core>\w+)/metadata/$', 'collections_fields_and_metadata', name='api_collections_metadata'),
  url(r'^api/collections/(?P<collection_or_core>\w+)/update/$', 'collections_update', name='api_collections_update'),
  url(r'^api/collections/(?P<collection_or_core>\w+)/data/$', 'collections_data', name='api_collections_data')
)
