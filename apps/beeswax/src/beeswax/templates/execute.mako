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
  from desktop.lib.django_util import extract_field_data
  from desktop.views import commonheader, commonfooter
  from django.utils.translation import ugettext as _
%>

<%namespace name="comps" file="beeswax_components.mako" />
<%namespace name="layout" file="layout.mako" />

${ commonheader(_('Query'), app_name, user) | n,unicode }
${layout.menubar(section='query')}

<div class="container-fluid">
  <div class="row-fluid">
    <div class="span2">
      <form id="advancedSettingsForm" action="${action}" method="POST" class="form form-horizontal">
        <div class="sidebar-nav">
          <ul class="nav nav-list">
            <li class="nav-header">${_('database')}</li>
            <li class="white">
              ${ form.query['database'] | n,unicode }
            </li>
            <li class="nav-header">${_('settings')}</li>
            <li class="white">
            % for i, f in enumerate(form.settings.forms):
            <div class="param">
                <div class="remove">
                    ${comps.field(f['_deleted'], tag="button", button_text="x", notitle=True, attrs=dict(
                        type="submit",
                        title=_("Delete this setting"),
                        klass="btn btn-mini settingsDelete"
                    ), value=True)}
                </div>

                <div class="control-group">
                    ${comps.label(f['key'])}
                    ${comps.field(f['key'], attrs={
                        'data-bind': "value: form['%s']" % f['key'].html_name,
                        'placeholder': app_name == 'impala' and "ABORT_ON_ERROR" or "mapred.reduce.tasks",
                        'klass': 'settingsField span8',
                        'autocomplete': 'off'
                      }
                    )}
                </div>

                <div class="control-group">
                    ${comps.label(f['value'])}
                    ${comps.field(f['value'], attrs=dict(
                        placeholder="1",
                        klass="span8"
                    ))}
                </div>
            </div>
            ${comps.field(f['_exists'], hidden=True)}

            % endfor
            <div class="control-group">
              <a class="btn btn-small" data-form-prefix="settings">${_('Add')}</a>
            </div>
            </li>
            <li class="nav-header
              % if app_name == 'impala':
                  hide
              % endif
              ">
            ${_('File Resources')}
            </li>
            <li class="white
              % if app_name == 'impala':
                  hide
              % endif
              ">
            % for i, f in enumerate(form.file_resources.forms):
            <div class="param">
                <div class="remove">
                    ${comps.field(f['_deleted'], tag="button", button_text="x", notitle=True, attrs=dict(
                        type="submit",
                        title=_("Delete this setting"),
                        klass="btn btn-mini file_resourcesDelete"
                    ), value=True)}
                </div>

                <div class="control-group">
                    ${comps.label(f['type'])}
                    ${comps.field(f['type'], render_default=True, attrs=dict(
                        klass="input-medium"
                    ))}
                </div>

                <div class="control-group">
                    ${comps.label(f['path'])}
                    ${comps.field(f['path'], attrs=dict(
                        placeholder="/user/foo/udf.jar",
                        klass="input-medium file_resourcesField pathChooser",
                        data_filters=f['path'].html_name
                    ))}
                </div>
            </div>
            ${comps.field(f['_exists'], hidden=True)}

            % endfor
                <div class="control-group">
                    <a class="btn btn-small" data-form-prefix="file_resources">${_('Add')}</a>
                </div>
            </li>
            <li class="nav-header
            % if app_name == 'impala':
                hide
            % endif
            ">
                ${_('User-defined Functions')}
            </li>
            <li class="white
            % if app_name == 'impala':
                hide
            % endif
            ">
                % for i, f in enumerate(form.functions.forms):
                    <div class="param">
                        <div class="remove">
                            ${comps.field(f['_deleted'], tag="button", button_text="x", notitle=True, attrs=dict(
                                type="submit",
                                title=_("Delete this setting"),
                                klass="btn btn-mini file_resourcesDelete"
                            ), value=True)}
                        </div>

                        <div class="control-group">
                            ${comps.label(f['name'])}
                            ${comps.field(f['name'], attrs=dict(
                                placeholder=_("myFunction"),
                                klass="span8 functionsField"
                            ))}
                        </div>

                        <div class="control-group">
                            ${comps.label(f['class_name'])}
                            ${comps.field(f['class_name'], attrs=dict(
                                placeholder="com.acme.example",
                                klass="span8"
                            ))}
                        </div>
                    </div>

                  ${comps.field(f['_exists'], hidden=True)}
                % endfor
                <div class="control-group">
                    <a class="btn btn-small" data-form-prefix="functions">${_('Add')}</a>
                </div>
            </li>
            <li class="nav-header">${_('Parameterization')}</li>
            <li class="white">
                <label class="checkbox" rel="tooltip" data-original-title="${_("If checked (the default), you can include parameters like $parameter_name in your query, and users will be prompted for a value when the query is run.")}">
                    <input type="checkbox" id="id_${form.query["is_parameterized"].html_name | n}" name="${form.query["is_parameterized"].html_name | n}" ${extract_field_data(form.query["is_parameterized"]) and "CHECKED" or ""}/>
                    ${_("Enable Parameterization")}
                </label>
            </li>
              <li class="nav-header
                % if app_name == 'impala':
                    hide
                % endif
              ">${_('Email Notification')}</li>
              <li class="white
                % if app_name == 'impala':
                    hide
                % endif
              ">
                <label class="checkbox" rel="tooltip" data-original-title="${_("If checked, you will receive an email notification when the query completes.")}">
                    <input type="checkbox" id="id_${form.query["email_notify"].html_name | n}" name="${form.query["email_notify"].html_name | n}" ${extract_field_data(form.query["email_notify"]) and "CHECKED" or ""}/>
                    ${_("Email me on completion.")}
                </label>
              </li>
            % if app_name == 'impala':
              <li class="nav-header">
                ${_('Metastore Catalog')}
              </li>
              <li class="white">
                <div class="control-group">
                  <span id="refresh-dyk">
                    <i class="fa fa-refresh"></i>
                    ${ _('Sync tables tips') }
                  </span>
                  <div id="refresh-content" class="hide">
                    <ul style="text-align: left;">
                      <li>"invalidate metadata" ${ _("invalidates the entire catalog metadata. All table metadata will be reloaded on the next access.") }</li>
                      <li>"invalidate metadata &lt;table&gt;" ${ _("invalidates the metadata, load on the next access") }</li>
                      <li>"refresh &lt;table&gt;" ${ _("refreshes the metadata immediately. It is a faster, incremental refresh.") }</li>
                    </ul>
                  </div>
                </div>
              </li>
            % endif
              <li class="nav-header"></li>
              <li class="white">
                <div class="control-group">
                  <i class="fa fa-question-circle" id="help"></i>
                  <div id="help-content" class="hide">
                    <ul style="text-align: left;">
                      <li>${ _('Press CTRL + Space to autocomplete') }</li>
                      <li>${ _("You can execute queries with multiple SQL statements delimited by a semicolon ';'") }</li>
                      <li>${ _('You can highlight and run a fragment of a query') }</li>
                    </ul>
                  </div>
                </div>
              </li>
           </ul>
            <input type="hidden" name="${form.query["query"].html_name | n}" class="query" value="" />
            </div>
                </form>
        </div>

        <div id="querySide" class="span8">
          <div class="card card-small">
            % if on_success_url:
              <input type="hidden" name="on_success_url" value="${on_success_url}"/>
            % endif
            <div style="margin-bottom: 30px">
              <h1 class="card-heading simple">
                ${ _('Query Editor') }
                % if can_edit_name:
                  :
                  <a href="#" id="query-name" data-type="text" data-pk="${ design.id }"
                     data-name="name"
                     data-url="${ url(app_name + ':save_design_properties') }"
                     data-original-title="${ _('Query name') }"
                     data-value="${ design.name }"
                     data-placement="left">
                  </a>
                %endif
              </h1>
              % if can_edit_name:
                <p style="margin-left: 20px">
                  <a href="#" id="query-description" data-type="textarea" data-pk="${ design.id }"
                     data-name="description"
                     data-url="${ url(app_name + ':save_design_properties') }"
                     data-original-title="${ _('Query description') }"
                     data-value="${ design.desc }"
                     data-placement="bottom">
                  </a>
                </p>
              % endif
            </div>
            <div class="card-body">
              <div class="tab-content">
                <div id="queryPane">

                  <div data-bind="css: {'hide': query.errors().length == 0}" class="hide alert alert-error">
                    <p><strong>${_('Your query has the following error(s):')}</strong></p>
                    <div data-bind="foreach: query.errors">
                      <p data-bind="text: $data" class="queryErrorMessage"></p>
                    </div>
                  </div>

                  <textarea class="hide" tabindex="1" name="query" id="queryField"></textarea>

                  <div class="actions">
                    <button data-bind="click: tryExecuteQuery" type="button" id="executeQuery" class="btn btn-primary" tabindex="2">${_('Execute')}</button>
                    <button data-bind="click: trySaveQuery, css: {'hide': !$root.query.id() || $root.query.id() == -1}" type="button" class="btn hide">${_('Save')}</button>
                    <button data-bind="click: trySaveAsQuery" type="button" class="btn">${_('Save as...')}</button>
                    <button data-bind="click: tryExplainQuery" type="button" id="explainQuery" class="btn">${_('Explain')}</button>
                    &nbsp; ${_('or create a')} &nbsp;<a type="button" class="btn" href="${ url('beeswax:execute_query') }">${_('New query')}</a>
                    <br /><br />
                </div>

                </div>
              </div>
            </div>
          </div>

          <div data-bind="css: {'hide': state() == 'full'}" class="hide">
           <div class="card card-small scrollable resultsContainer">
              <!-- ko if: state() == 'watching' -->
                <h1 class="card-heading simple">${_('Waiting for query...')}</h1>
              <!-- /ko -->
              <!-- ko if: state() == 'results' -->
                <h1 class="card-heading simple">${_('Results for query available.')}</h1>
              <!-- /ko -->
              <div class="card-body">
                <ul class="nav nav-tabs">
                  <!-- ko if: state() != 'explanation' -->
                  <li><a href="#log" data-toggle="tab">${_('Log')}</a></li>
                  <!-- /ko -->
                  <li><a href="#query" data-toggle="tab">${_('Query')}</a></li>
                  <!-- ko if: state() != 'explanation' -->
                  <li><a href="#results" data-toggle="tab">${_('Results')}</a></li>
                  <!-- /ko -->
                  <!-- ko if: state() == 'explanation' -->
                  <li><a href="#explanation" data-toggle="tab">${_('Explanation')}</a></li>
                  <!-- /ko -->
                </ul>

                <div class="tab-content">
                  <div class="tab-pane" id="query">
                    <pre data-bind="text: viewModel.query.query()"></pre>
                  </div>
                  <!-- ko if: state() == 'explanation' -->
                  <div class="tab-pane" id="explanation">
                    <pre data-bind="text: viewModel.explanation()"></pre>
                  </div>
                  <!-- /ko -->
                  <!-- ko if: state() != 'explanation' -->
                  <div class="active tab-pane" id="log">
                    <pre data-bind="text: viewModel.logs().join('\n')"></pre>
                  </div>
                  <div class="tab-pane" id="results">
                    <div data-bind="css: {'hide': rows().length == 0}" class="hide scrollable">
                      <table class="table table-striped table-condensed resultTable" cellpadding="0" cellspacing="0" data-tablescroller-min-height-disable="true" data-tablescroller-enforce-height="true">
                        <thead>
                          <tr data-bind="foreach: columns">
                            <th data-bind="text: $data"></th>
                          </tr>
                        </thead>
                      </table>
                    </div>

                    <div data-bind="css: {'hide': !resultsEmpty()}" class="hide">
                      <div class="card card-small scrollable">
                        <div class="row-fluid">
                          <div class="span10 offset1 center empty-wrapper">
                            <i class="fa fa-frown-o"></i>
                            <h1>${_('The server returned no results.')}</h1>
                            <br />
                          </div>
                        </div>
                      </div>
                    </div>
                  </div>
                  <!-- /ko -->
                </div>
              </div>
            </div>
          </div>
      </div>

    <div class="span2" id="navigator">
      <div class="card card-small">
        <a href="#" title="${_('Double click on a table name or field to insert it in the editor')}" rel="tooltip" data-placement="left" class="pull-right" style="margin:10px;margin-left: 0"><i class="fa fa-question-circle"></i></a>
        <a id="refreshNavigator" href="#" title="${_('Manually refresh the table list')}" rel="tooltip" data-placement="left" class="pull-right" style="margin:10px"><i class="fa fa-refresh"></i></a>
        <h1 class="card-heading simple"><i class="fa fa-compass"></i> ${_('Navigator')}</h1>
        <div class="card-body">
          <p>
            <input id="navigatorSearch" type="text" placeholder="${ _('Table name...') }" style="width:90%"/>
            <span id="navigatorNoTables">${_('The selected database has no tables.')}</span>
            <ul id="navigatorTables" class="unstyled"></ul>
            <div id="navigatorLoader">
              <!--[if !IE]><!--><i class="fa fa-spinner fa-spin" style="font-size: 20px; color: #DDD"></i><!--<![endif]-->
              <!--[if IE]><img src="/static/art/spinner.gif" /><![endif]-->
            </div>
          </p>
        </div>
      </div>
    </div>
  </div>
</div>


<div id="chooseFile" class="modal hide fade">
    <div class="modal-header">
        <a href="#" class="close" data-dismiss="modal">&times;</a>
        <h3>${_('Choose a file')}</h3>
    </div>
    <div class="modal-body">
        <div id="filechooser">
        </div>
    </div>
    <div class="modal-footer">
    </div>
</div>

<div id="saveAs" class="modal hide fade">
    <div class="modal-header">
        <a href="#" class="close" data-dismiss="modal">&times;</a>
        <h3>${_('Choose a name')}</h3>
    </div>
      <form class="form-horizontal">
        <div class="control-group" id="saveas-query-name">
          <label class="control-label">${_('Name')}</label>
          <div class="controls">
            <input data-bind="value: $root.query.name" type="text" class="input-xlarge">
          </div>
        </div>
        <div class="control-group">
          <label class="control-label">${_('Description')}</label>
          <div class="controls">
            <input data-bind="value: $root.query.description" type="text" class="input-xlarge">
          </div>
        </div>
      </form>
    </div>
    <div class="modal-footer">
        <button class="btn" data-dismiss="modal">${_('Cancel')}</button>
        <button id="saveAsNameBtn" class="btn btn-primary">${_('Save')}</button>
    </div>
</div>

<div id="navigatorQuicklook" class="modal hide fade">
    <div class="modal-header">
        <a href="#" class="close" data-dismiss="modal">&times;</a>
        <a class="tableLink pull-right" href="#" target="_blank" style="margin-right: 20px;margin-top:6px"><i class="fa fa-external-link"></i> ${ _('View in Metastore Browser') }</a>
        <h3>${_('Data sample for')} <span class="tableName"></span></h3>
    </div>
    <div class="modal-body" style="min-height: 100px">
      <div class="loader">
        <!--[if !IE]><!--><i class="fa fa-spinner fa-spin" style="font-size: 30px; color: #DDD"></i><!--<![endif]-->
        <!--[if IE]><img src="/static/art/spinner.gif" /><![endif]-->
      </div>
      <div class="sample"></div>
    </div>
    <div class="modal-footer">
        <button class="btn btn-primary disable-feedback" data-dismiss="modal">${_('Ok')}</button>
    </div>
</div>

<script src="/static/ext/js/knockout-min.js" type="text/javascript" charset="utf-8"></script>
<script src="/static/ext/js/knockout.mapping-2.3.2.js" type="text/javascript" charset="utf-8"></script>
<script src="/beeswax/static/js/beeswax.vm.js"></script>
<script src="/static/ext/js/codemirror-3.11.js"></script>
<link rel="stylesheet" href="/static/ext/css/codemirror.css">
<script src="/static/js/codemirror-hql.js"></script>
% if app_name == 'impala':
  <script src="/static/js/codemirror-isql-hint.js"></script>
% else:
  <script src="/static/js/codemirror-hql-hint.js"></script>
% endif
<script src="/static/js/codemirror-show-hint.js"></script>

<link href="/static/ext/css/bootstrap-editable.css" rel="stylesheet">
<script src="/static/ext/js/bootstrap-editable.min.js"></script>

<style type="text/css">
  h1 {
    margin-bottom: 5px;
  }
  #filechooser {
    min-height: 100px;
    overflow-y: auto;
  }

  .control-group {
    margin-bottom: 3px!important;
  }

  .control-group label {
    float: left;
    padding-top: 5px;
    text-align: left;
    width: 40px;
  }

  .sidebar-nav {
    margin-bottom: 90px !important;
  }

  .param {
    padding: 8px 8px 1px 8px;
    margin-bottom: 5px;
    border-bottom: 1px solid #EEE;
  }

  .remove {
    float: right;
  }

  .file_resourcesField {
    border-radius: 3px 0 0 3px;
    border-right: 0!important;
    min-height: 27px!important;
  }

  .fileChooserBtn {
    border-radius: 0 3px 3px 0;
  }

  .CodeMirror {
    border: 1px solid #eee;
    margin-bottom: 20px;
  }

  .editorError {
    color: #B94A48;
    background-color: #F2DEDE;
    padding: 4px;
    font-size: 11px;
  }

  .editable-empty, .editable-empty:hover {
    color: #666;
    font-style: normal;
  }

  #navigatorTables li {
    width: 95%;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }

  #navigatorSearch, #navigatorNoTables {
    display: none;
  }

  .tooltip.left {
    margin-left: -13px;
  }

</style>

<script src="/static/ext/js/jquery/plugins/jquery-fieldselection.js" type="text/javascript"></script>
<script src="/beeswax/static/js/autocomplete.utils.js" type="text/javascript" charset="utf-8"></script>

<script type="text/javascript" charset="utf-8">
  var codeMirror, renderNavigator, resetNavigator;

  var HIVE_AUTOCOMPLETE_BASE_URL = "${ autocomplete_base_url | n,unicode }";
  var HIVE_AUTOCOMPLETE_FAILS_SILENTLY_ON = [500]; // error codes from beeswax/views.py - autocomplete

  var HIVE_AUTOCOMPLETE_GLOBAL_CALLBACK = function (data) {
    if (data != null && data.error) {
      resetNavigator();
    }
  };

  $(document).ready(function(){

    ## If no particular query is loaded
    % if design is None or design.id is None:
      if ($.cookie("hueBeeswaxLastDatabase") != null) {
        $("#id_query-database").val($.cookie("hueBeeswaxLastDatabase"));
      }
    % endif

    $("#navigatorQuicklook").modal({
      show: false
    });

    var navigatorSearchTimeout = -1;
    $("#navigatorSearch").on("keyup", function () {
      window.clearTimeout(navigatorSearchTimeout);
      navigatorSearchTimeout = window.setTimeout(function () {
        $("#navigatorTables li").removeClass("hide");
        $("#navigatorTables li").each(function () {
          if ($(this).text().toLowerCase().indexOf($("#navigatorSearch").val().toLowerCase()) == -1) {
            $(this).addClass("hide");
          }
        });
      }, 300);
    });

    resetNavigator = function () {
      var _db = $("#id_query-database").val();
      if (_db != null) {
        $.totalStorage('tables_' + _db, null);
        $.totalStorage('timestamp_tables_' + _db, null);
        renderNavigator();
      }
    }

    renderNavigator = function () {
      $("#navigatorTables").empty();
      $("#navigatorLoader").show();
      hac_getTables($("#id_query-database").val(), function (data) {  //preload tables for the default db
        $(data.split(" ")).each(function (cnt, table) {
          if ($.trim(table) != ""){
            var _table = $("<li>");
            _table.html("<a href='/metastore/table/" + $("#id_query-database").val() + "/" + table + "' target='_blank' class='pull-right'><i class='fa fa-eye' title='" + "${ _('View in Metastore Browser') }" + "' style='margin-left:5px'></i></a><a href='#' class='pull-right hide'><i class='fa fa-list' title='" + "${ _('Preview Sample data') }" + "'></i></a><a href='#' title='" + table + "'><i class='fa fa-table'></i> " + table + "</a><ul class='unstyled'></ul>");
            _table.data("table", table).attr("id", "navigatorTables_" + table);
            _table.find("a:eq(2)").on("click", function () {
              _table.find(".fa-table").removeClass("fa-table").addClass("fa-spin").addClass("fa-spinner");
              hac_getTableColumns($("#id_query-database").val(), table, "", function (plain_columns, extended_columns) {
                _table.find("a:eq(1)").removeClass("hide");
                _table.find("ul").empty();
                _table.find(".fa-spinner").removeClass("fa-spinner").removeClass("fa-spin").addClass("fa-table");
                $(extended_columns).each(function (iCnt, col) {
                  var _column = $("<li>");
                  _column.html("<a href='#' style='padding-left:10px'" + (col.comment != null && col.comment != "" ? " title='" + col.comment + "'" : "") + "><i class='fa fa-columns'></i> " + col.name + " (" + col.type + ")</a>");
                  _column.appendTo(_table.find("ul"));
                  _column.on("dblclick", function () {
                    codeMirror.replaceSelection($.trim(col.name) + ', ');
                    codeMirror.setSelection(codeMirror.getCursor());
                    codeMirror.focus();
                  });
                });
              });
            });
            _table.find("a:eq(2)").on("dblclick", function () {
              codeMirror.replaceSelection($.trim(table) + ' ');
              codeMirror.setSelection(codeMirror.getCursor());
              codeMirror.focus();
            });
            _table.find("a:eq(1)").on("click", function () {
              $("#navigatorQuicklook").find(".tableName").text(table);
              $("#navigatorQuicklook").find(".tableLink").attr("href", "/metastore/table/" + $("#id_query-database").val() + "/" + _table.data("table"));
              $("#navigatorQuicklook").find(".sample").empty("");
              $("#navigatorQuicklook").attr("style", "width: " + ($(window).width() - 120) + "px;margin-left:-" + (($(window).width() - 80) / 2) + "px!important;");
              $.ajax({
                url: "/metastore/table/" + $("#id_query-database").val() + "/" + _table.data("table"),
                data: {"sample": true},
                beforeSend: function (xhr) {
                  xhr.setRequestHeader("X-Requested-With", "Hue");
                },
                dataType: "html",
                success: function (data) {
                  $("#navigatorQuicklook").find(".loader").hide();
                  $("#navigatorQuicklook").find(".sample").html(data);
                },
                error: function (e) {
                  if (e.status == 500){
                    resetNavigator();
                    $(document).trigger("error", "${ _('There was a problem loading the table preview.') }");
                    $("#navigatorQuicklook").modal("hide");
                  }
                }
              });
              $("#navigatorQuicklook").modal("show");
            });
            _table.appendTo($("#navigatorTables"));
          }
        });
        $("#navigatorLoader").hide();
        if ($("#navigatorTables li").length > 0) {
          $("#navigatorSearch").show();
          $("#navigatorNoTables").hide();
        }
        else {
          $("#navigatorSearch").hide();
          $("#navigatorNoTables").show();
        }
      });
    }

    renderNavigator();

    $("#refreshNavigator").on("click", function(){
      resetNavigator();
    });


    $("#navigator .card").css("min-height", ($(window).height() - 130) + "px");
    $("#navigatorTables").css("max-height", ($(window).height() - 260) + "px").css("overflow-y", "auto");

    var queryPlaceholder = "${_('Example: SELECT * FROM tablename, or press CTRL + space')}";

    var successfunction = function (response, newValue) {
      if (response.status != 0) {
        $(document).trigger("error", "${ _('Problem: ') }" + response.data);
      }
    }

    $("#query-name").editable({
      validate: function (value) {
        if ($.trim(value) == '') {
          return "${ _('This field is required.') }";
        }
      },
      success: successfunction,
      emptytext: "${ _('Query name') }"
    });

    $("#query-description").editable({
      success: successfunction,
      emptytext: "${ _('Empty description') }"
    });

    $("*[rel=tooltip]").tooltip({
      placement: 'bottom'
    });

    $("a[data-form-prefix]").each(function () {
      var _prefix = $(this).attr("data-form-prefix");
      var _nextID = 0;
      if ($("." + _prefix + "Field").length) {
        _nextID = ($("." + _prefix + "Field").last().attr("name").substr(_prefix.length + 1).split("-")[0] * 1) + 1;
      }
      $("<input>").attr("type", "hidden").attr("name", _prefix + "-next_form_id").attr("value", _nextID).appendTo($("#advancedSettingsForm"));
      $("." + _prefix + "Delete").click(function (e) {
        e.preventDefault();
        $("input[name=" + _prefix + "-add]").attr("value", "");
        $("<input>").attr("type", "hidden").attr("name", $(this).attr("name")).attr("value", "True").appendTo($("#advancedSettingsForm"));
        tryExecuteQuery();
      });
    });

    $("a[data-form-prefix]").click(function () {
      var _prefix = $(this).attr("data-form-prefix");
      $("<input>").attr("type", "hidden").attr("name", _prefix + "-add").attr("value", "True").appendTo($("#advancedSettingsForm"));
      tryExecuteQuery();
    });

    $(".file_resourcesField").each(function () {
      var self = $(this);
      self.after(getFileBrowseButton(self));
    });

    function getFileBrowseButton(inputElement) {
      return $("<button>").addClass("btn").addClass("fileChooserBtn").text("..").click(function (e) {
        e.preventDefault();
        $("#filechooser").jHueFileChooser({
          initialPath: inputElement.val(),
          onFileChoose: function (filePath) {
            inputElement.val(filePath);
            $("#chooseFile").modal("hide");
          },
          createFolder: false
        });
        $("#chooseFile").modal("show");
      })
    }

    $("#id_query-database").change(function () {
      $.cookie("hueBeeswaxLastDatabase", $(this).val(), {expires: 90});
      renderNavigator();
    });

    var executeQuery = function () {
      $("input[name='button-explain']").remove();
      $("<input>").attr("type", "hidden").attr("name", "button-submit").attr("value", "Execute").appendTo($("#advancedSettingsForm"));
      tryExecuteQuery();
    }

    // $("#executeQuery").click(executeQuery);
    $("#executeQuery").tooltip({
      title: '${_("Press \"tab\", then \"enter\".")}'
    });
    $("#executeQuery").keyup(function (event) {
      if (event.keyCode == 13) {
        executeQuery();
      }
    });

    % if app_name == 'impala':
      $("#downloadQuery").click(function () {
        $("<input>").attr("type", "hidden").attr("name", "button-submit").attr("value", "Execute").appendTo($("#advancedSettingsForm"));
        $("<input>").attr("type", "hidden").attr("name", "download").attr("value", "true").appendTo($("#advancedSettingsForm"));
        tryExecuteQuery();
      });
    % endif

    $("#saveQuery").click(function () {
      $("<input>").attr("type", "hidden").attr("name", "saveform-name")
              .attr("value", $("#query-name").text()).appendTo($("#advancedSettingsForm"));
      var description = $("#query-description").text() != "${ _('Empty description') }" ? $("#query-description").text() : "";
      $("<input>").attr("type", "hidden").attr("name", "saveform-desc")
              .attr("value", description).appendTo($("#advancedSettingsForm"));
      $("<input>").attr("type", "hidden").attr("name", "saveform-save").attr("value", "Save").appendTo($("#advancedSettingsForm"));
      tryExecuteQuery();
    });

    $("#saveQueryAs").click(function () {
      $("<input>").attr("type", "hidden").attr("name", "saveform-saveas").attr("value", "Save As...").appendTo($("#advancedSettingsForm"));
      $("#saveAs").modal("show");
    });

    $("#saveAsNameBtn").click(function () {
      $("<input>").attr("type", "hidden").attr("name", "saveform-name")
              .attr("value", $("input[name='saveform-name']").val()).appendTo($("#advancedSettingsForm"));
      $("<input>").attr("type", "hidden").attr("name", "saveform-desc")
              .attr("value", $("input[name='saveform-desc']").val()).appendTo($("#advancedSettingsForm"));
      tryExecuteQuery();
    });

    $("#explainQuery").click(function () {
      $("input[name='button-execute']").remove();
    });

    initQueryField();

    var resizeTimeout = -1;
    var winWidth = $(window).width();
    var winHeight = $(window).height();

    $(window).on("resize", function () {
      window.clearTimeout(resizeTimeout);
      resizeTimeout = window.setTimeout(function () {
        // prevents endless loop in IE8
        if (winWidth != $(window).width() || winHeight != $(window).height()) {
          codeMirror.setSize("95%", 100);
          $("#navigator .card").css("min-height", ($(window).height() - 130) + "px");
          $("#navigatorTables").css("max-height", ($(window).height() - 260) + "px").css("overflow-y", "auto");
          winWidth = $(window).width();
          winHeight = $(window).height();
        }
      }, 200);
    });

    function initQueryField() {
      if ($("#queryField").val() == "") {
        $("#queryField").val(queryPlaceholder);
      }
    }

    var queryEditor = $("#queryField")[0];

    % if app_name == 'impala':
      var AUTOCOMPLETE_SET = CodeMirror.impalaSQLHint;
    % else:
      var AUTOCOMPLETE_SET = CodeMirror.hiveQLHint;
    % endif

    CodeMirror.onAutocomplete = function (data, from, to) {
      if (CodeMirror.tableFieldMagic) {
        codeMirror.replaceRange(" ", from, from);
        codeMirror.setCursor(from);
        codeMirror.execCommand("autocomplete");
      }
    };

    CodeMirror.commands.autocomplete = function (cm) {
      $(document.body).on("contextmenu", function (e) {
        e.preventDefault(); // prevents native menu on FF for Mac from being shown
      });

      var pos = cm.cursorCoords();
      $("<i class='fa fa-spinner fa-spin CodeMirror-spinner'></i>").css("top", pos.top + "px").css("left", (pos.left - 4) + "px").appendTo($("body"));

      if ($.totalStorage('tables_' + $("#id_query-database").val()) == null) {
        CodeMirror.showHint(cm, AUTOCOMPLETE_SET);
        hac_getTables($("#id_query-database").val(), function () {}); // if preload didn't work, tries again
      }
      else {
        hac_getTables($("#id_query-database").val(), function (tables) {
          CodeMirror.catalogTables = tables;
          var _before = codeMirror.getRange({line: 0, ch: 0}, {line: codeMirror.getCursor().line, ch: codeMirror.getCursor().ch}).replace(/(\r\n|\n|\r)/gm, " ");
          CodeMirror.possibleTable = false;
          CodeMirror.tableFieldMagic = false;
          if (_before.toUpperCase().indexOf(" FROM ") > -1 && _before.toUpperCase().indexOf(" ON ") == -1 && _before.toUpperCase().indexOf(" WHERE ") == -1 ||
              _before.toUpperCase().indexOf("REFRESH") > -1 || _before.toUpperCase().indexOf("METADATA") > -1 ) {
            CodeMirror.possibleTable = true;
          }
          CodeMirror.possibleSoloField = false;
          if (_before.toUpperCase().indexOf("SELECT ") > -1 && _before.toUpperCase().indexOf(" FROM ") == -1 && !CodeMirror.fromDot) {
            if (codeMirror.getValue().toUpperCase().indexOf("FROM ") > -1) {
              fieldsAutocomplete(cm);
            }
            else {
              CodeMirror.tableFieldMagic = true;
              CodeMirror.showHint(cm, AUTOCOMPLETE_SET);
            }
          }
          else {
            if (_before.toUpperCase().indexOf("WHERE ") > -1 && !CodeMirror.fromDot && _before.match(/ON|GROUP|SORT/) == null) {
              fieldsAutocomplete(cm);
            }
            else {
              CodeMirror.showHint(cm, AUTOCOMPLETE_SET);
            }
          }
        });
      }
    }

    function fieldsAutocomplete(cm) {
      CodeMirror.possibleSoloField = true;
      try {
        var _possibleTables = $.trim(codeMirror.getValue(" ").substr(codeMirror.getValue().toUpperCase().indexOf("FROM ") + 4)).split(" ");
        var _foundTable = "";
        for (var i = 0; i < _possibleTables.length; i++) {
          if ($.trim(_possibleTables[i]) != "" && _foundTable == "") {
            _foundTable = _possibleTables[i];
          }
        }
        if (_foundTable != "") {
          if (hac_tableHasAlias(_foundTable, codeMirror.getValue())) {
            CodeMirror.possibleSoloField = false;
            CodeMirror.showHint(cm, AUTOCOMPLETE_SET);
          }
          else {
            hac_getTableColumns($("#id_query-database").val(), _foundTable, codeMirror.getValue(),
                function (columns) {
                  CodeMirror.catalogFields = columns;
                  CodeMirror.showHint(cm, AUTOCOMPLETE_SET);
                });
          }
        }
      }
      catch (e) {
      }
    }

    CodeMirror.fromDot = false;

    codeMirror = CodeMirror(function (elt) {
      queryEditor.parentNode.replaceChild(elt, queryEditor);
    }, {
      value: queryEditor.value,
      readOnly: false,
      lineNumbers: true,
      mode: "text/x-hiveql",
      extraKeys: {
        "Ctrl-Space": function () {
          CodeMirror.fromDot = false;
          codeMirror.execCommand("autocomplete");
        },
        Tab: function (cm) {
          $("#executeQuery").focus();
        }
      },
      onKeyEvent: function (e, s) {
        if (s.type == "keyup") {
          if (s.keyCode == 190) {
            var _line = codeMirror.getLine(codeMirror.getCursor().line);
            var _partial = _line.substring(0, codeMirror.getCursor().ch);
            var _table = _partial.substring(_partial.lastIndexOf(" ") + 1, _partial.length - 1);
            if (codeMirror.getValue().toUpperCase().indexOf("FROM") > -1) {
              hac_getTableColumns($("#id_query-database").val(), _table, codeMirror.getValue(), function (columns) {
                var _cols = columns.split(" ");
                for (var col in _cols){
                  _cols[col] = "." + _cols[col];
                }
                CodeMirror.catalogFields = _cols.join(" ");
                CodeMirror.fromDot = true;
                window.setTimeout(function () {
                  codeMirror.execCommand("autocomplete");
                }, 100);  // timeout for IE8
              });
            }
          }
        }
      }
    });

    var selectedLine = -1;
    var errorWidget = null;
    if ($(".queryErrorMessage").length > 0) {
      var err = $(".queryErrorMessage").text().toLowerCase();
      var firstPos = err.indexOf("line");
      if (firstPos > -1) {
        selectedLine = $.trim(err.substring(err.indexOf(" ", firstPos), err.indexOf(":", firstPos))) * 1;
        errorWidget = codeMirror.addLineWidget(selectedLine - 1, $("<div>").addClass("editorError").html("<i class='fa fa-exclamation-circle'></i> " + err)[0], {coverGutter: true, noHScroll: true})
      }
    }

    codeMirror.setSize("95%", $(window).height() - 270 - $("#queryPane .alert-error").outerHeight() - $(".nav-tabs").outerHeight());

    codeMirror.on("focus", function () {
      if (codeMirror.getValue() == queryPlaceholder) {
        codeMirror.setValue("");
      }
      if (errorWidget) {
        errorWidget.clear();
      }
      $("#validationResults").empty();
    });

    % if design and not design.id:
    if ($.totalStorage("${app_name}_temp_query") != null && $.totalStorage("${app_name}_temp_query") != "") {
      codeMirror.setValue($.totalStorage("${app_name}_temp_query"));
    }
    % endif

    codeMirror.on("blur", function () {
      $(document.body).off("contextmenu");
    });

    codeMirror.on("change", function () {
      $(".query").val(codeMirror.getValue());
      $.totalStorage("${app_name}_temp_query", codeMirror.getValue());
    });

    % if app_name == 'impala':
      $("#refresh-dyk").popover({
        'title': "${_('Missing some tables? In order to update the list of tables/metadata seen by Impala, execute one of these queries:')}",
        'content': $("#refresh-content").html(),
        'trigger': 'hover',
        'html': true
      });

      $("#refresh-tip").popover({
        'title': "${_('Missing some tables? In order to update the list of tables/metadata seen by Impala, execute one of these queries:')}",
        'content': $("#refresh-content").html(),
        'trigger': 'hover',
        'html': true
      });
    % endif
  });

  $.getJSON("${ url(app_name + ':configuration') }", function(data) {
    $(".settingsField").typeahead({
      source: $.map(data.config_values, function(value, key) {
        return value.key;
      })
    });
  });

  $("#help").popover({
      'title': "${_('Did you know?')}",
      'content': $("#help-content").html(),
      'trigger': 'hover',
      'html': true
  });




  $(document).ready(function(){
    var labels = {
      MRJOB: "${_('MR Job')}",
      MRJOBS: "${_('MR Jobs')}"
    }

    var logsAtEnd = true;

    $(window).resize(function () {
      resizeLogs();
    });

    $("a[href='#log']").on("shown", function () {
      resizeLogs();
    });

    $("#log pre").scroll(function () {
      if ($(this).scrollTop() + $(this).height() + 20 >= $(this)[0].scrollHeight) {
        logsAtEnd = true;
      }
      else {
        logsAtEnd = false;
      }
    });

    function resizeLogs() {
      // Use fixed subtraction since logs aren't always visible.
      $("#log pre").css("overflow", "auto").height($(window).height() - 557);
    }
  });

  var dataTable = null;
  function cleanResultsTable() {
    if (dataTable) {
      dataTable.fnClearTable();
      dataTable.fnDestroy();
      viewModel.columns.valueHasMutated();
      viewModel.rows.valueHasMutated();
      dataTable = null;
    }
  }

  function addResults(viewModel, dataTable, index, pageSize) {
    if (viewModel.hasMoreResults() && index + pageSize > viewModel.rows().length) {
      $(document).one('fetched.results', function() {
        dataTable.fnAddData(viewModel.rows.slice(index, index+pageSize));
      });
      viewModel.fetchResults();
    } else {
      dataTable.fnAddData(viewModel.rows.slice(index, index+pageSize));
    }
  }

  function resultsTable() {
    if (!dataTable) {
      dataTable = $(".resultTable").dataTable({
        "bPaginate": false,
        "bLengthChange": false,
        "bInfo": false,
        "oLanguage": {
          "sEmptyTable": "${_('No data available')}",
          "sZeroRecords": "${_('No matching records')}"
        },
        "fnDrawCallback": function( oSettings ) {
          $(".resultTable").jHueTableExtender({
            hintElement: "#jumpToColumnAlert",
            fixedHeader: true,
            firstColumnTooltip: true
          });
        },
        "fnRowCallback": function (nRow, aData, iDisplayIndex, iDisplayIndexFull) {
          // Make sure null values are seen as NULL.
          for(var j = 0; j < aData.length; ++j) {
            if (aData[j] == null) {
              $(nRow).find('td:eq('+j+')').html("NULL");
            }
          }
          return nRow;
        }
      });
      $(".dataTables_filter").hide();
      $(".dataTables_wrapper").jHueTableScroller();

      // Automatic results grower
      var dataTableEl = $(".dataTables_wrapper");
      var index = 0;
      var pageSize = 100;
      dataTableEl.on("scroll", function (e) {
        if (dataTableEl.scrollTop() + dataTableEl.outerHeight() + 20 > dataTableEl[0].scrollHeight && dataTable) {
          dataTableEl.animate({opacity: '0.55'}, 200);
          $(".spinner").show();
          addResults(viewModel, dataTable, index, pageSize);
          index += pageSize;
          $(".spinner").hide();
          dataTableEl.animate({opacity: '1'}, 50);
        }
      });
      addResults(viewModel, dataTable, index, pageSize);
      index += pageSize;

      $(".resultTable").width($(".resultTable").parent().width());
    }
  }

  $(document).on('execute.query', cleanResultsTable);
  $(document).on('explain.query', cleanResultsTable);
  $(document).on('fetched.results', resultsTable);



  // hack for select default rendered fields
  $("select").addClass("input-medium");

  function getHighlightedQuery() {
    var selection = codeMirror.getSelection();
    if (selection != "") {
      return selection;
    }
    return null;
  }

  function tryExecuteQuery() {
    var query = getHighlightedQuery() || codeMirror.getValue();
    viewModel.query.query(query);
    viewModel.executeQuery();
    codeMirror.setSize("95%", 100);
  }

  function tryExplainQuery() {
    var query = getHighlightedQuery() || codeMirror.getValue();
    viewModel.query.query(query);
    viewModel.explainQuery();
    codeMirror.setSize("95%", 100);
  }

  function trySaveQuery() {
    var query = getHighlightedQuery() || codeMirror.getValue();
    viewModel.query.query(query);
    if (viewModel.query.id() && viewModel.query.id() != -1) {
      viewModel.saveQuery();
    }
  }

  function trySaveAsQuery() {
    var query = getHighlightedQuery() || codeMirror.getValue();
    viewModel.query.query(query);
    $('#saveAsQueryModal').modal('show');
  }

  function modalSaveAsQuery() {
    if (viewModel.query.query() && viewModel.query.name()) {
      viewModel.query.id(-1);
      viewModel.saveQuery();
      $('#saveas-query-name').removeClass('error');
      $('#saveAsQueryModal').modal('hide');
    } else if (viewModel.query.name()) {
      $.jHueNotify.error("${_('No query provided to save.')}");
      $('#saveAsQueryModal').modal('hide');
    } else {
      $('#saveas-query-name').addClass('error');
    }
  }

  function checkLastDatabase(server, database) {
    var key = "hueBeeswaxLastDatabase-" + server;
    if (database != $.totalStorage(key)) {
      $.totalStorage(key, database);
    }
  }

  function getLastDatabase(server) {
    var key = "hueBeeswaxLastDatabase-" + server;
    return $.totalStorage(key);
  }

  function clickHard(el) {
    var timer = setInterval(function() {
      if ($(el).length > 0) {
        $(el).click();
        clearInterval(timer);
      }
    }, 100);
  }



  // Knockout
  viewModel = new BeeswaxViewModel("${app_name}");
  viewModel.fetchDatabases();
  viewModel.state.subscribe(function(state) {
    // Switch between tabs automatically
    if (state == 'watching') {
      clickHard('.resultsContainer .nav-tabs a[href="#log"]');
    } else if (state == 'results') {
      clickHard('.resultsContainer .nav-tabs a[href="#results"]');
    } else if (state == 'explanation') {
      clickHard('.resultsContainer .nav-tabs a[href="#explanation"]');
    }
  });
  ko.applyBindings(viewModel);

  // Server error handling.
  $(document).on('server.error', function(e, data) {
    $(document).trigger('error', "${_('Server error occured: ')}" + data.error);
  });
  $(document).on('server.unmanageable_error', function(e, responseText) {
    $(document).trigger('error', "${_('Unmanageable server error occured: ')}" + responseText);
  });

  // @TODO: Improve resize logs to be more relative. See FF versus Chrome.
  // @TODO: Cancel operation for impala.
  // @TODO: Save
  // @TODO: Save As
  // @TODO: Close
  // @TODO: Stop operation
  // @TODO: Enable/Disable parameterization
  // @TODO: Email notification
</script>

${ commonfooter(messages) | n,unicode }
