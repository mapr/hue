## Licensed to Cloudera, Inc. under one
## or more contributor license agreements.  See the NOTICE file
## distributed with this work for additional information
## regarding copyright ownership.  Cloudera, Inc. licenses this file
## to you under the Apache License, Version 2.0 (the
## "License"); you may not use this file except in compliance
## with the License.  You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
<%!
  from filebrowser.views import location_to_url
  from desktop.lib.i18n import smart_unicode
  from desktop.views import commonheader, commonfooter
  from django.utils.translation import ugettext as _
%>

<%namespace name="components" file="components.mako" />

${ commonheader(_('Partition: %(partition_id)s') % dict(partition_id=partition_id), app_name, user) | n,unicode }
${ components.menubar() }

<div class="container-fluid">
  <div class="row-fluid">
    <div class="span3">
      <div class="sidebar-nav card-small">
        <ul class="nav nav-list">
          <li class="nav-header">${_('Actions')}</li>
          <li><a href="${ url('metastore:describe_partitions', database=database, table=table) }"><i class="fa fa-reply"></i> ${_('Show Partitions')}</a></li>
        </ul>
      </div>
    </div>
    <div class="span9">
      <div class="card card-small">
        <h1 class="card-heading simple">${ components.breadcrumbs(breadcrumbs) }</h1>
          <div class="card-body">
            <p>
            % if properties:
                <table class="table table-striped table-condensed">
                  <thead>
                    <tr>
                      <th>${ _('Name') }</th>
                      <th>${ _('Value') }</th>
                      <th>${ _('Comment') }</th>
                    </tr>
                  </thead>
                  <tbody>
                    % for prop in properties:
                      <tr>
                        <td>${ smart_unicode(prop['col_name']) }</td>
                        <td>${ smart_unicode(prop['data_type']) if prop['data_type'] else '' }</td>
                        <td>${ smart_unicode(prop['comment']) if prop['comment'] else '' }&nbsp;</td>
                      </tr>
                     % endfor
                  </tbody>
                </table>
            % else:
              <div class="alert">${_('The partition %s has no properties.' % partition_id)}</div>
            % endif
            </p>
          </div>
        </div>
    </div>
  </div>
</div>

<link rel="stylesheet" href="${ static('metastore/css/metastore.css') }" type="text/css">

<script type="text/javascript" charset="utf-8">
  $(document).ready(function () {
    $("a[data-row-selector='true']").jHueRowSelector();
  });
</script>

${ commonfooter(messages) | n,unicode }
