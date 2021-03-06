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
  from desktop.views import commonheader, commonfooter
  from desktop import conf
  from django.utils.translation import ugettext as _
%>

<%namespace name="koComponents" file="/ko_components.mako" />
<%namespace name="editorComponents" file="editor_components.mako" />

${ commonheader(_('Editor'), app_name, user, "68px") | n,unicode }

<script type="text/javascript">
  if (window.location.hash != "") {
    if (window.location.hash.indexOf("editor") > -1) {
      location.href = "/spark/editor?" + window.location.hash.substr(1).replace(/(<([^>]+)>)/ig, "");
    }
  }
</script>


${ editorComponents.includes() }
${ editorComponents.commonHTML() }


${ koComponents.csvListInput() }
${ koComponents.jvmMemoryInput() }
${ koComponents.assistPanel() }

${ editorComponents.commonJS() }

<script type="text/javascript" charset="utf-8">

  var aceAutocomplete = new Autocomplete({
    autocompleteBaseURL: "${ autocomplete_base_url | n,unicode }",
    autocompleteApp: "beeswax",
    autocompleteUser: "${user}",
    autocompleteFailsQuietlyOn: [500] // error codes from beeswax/views.py - autocomplete
  });

  var assist = new Assist({
    app: "beeswax",
    user: "${user}",
    failsSilentlyOn: [500], // error codes from beeswax/views.py - autocomplete
    baseURL: "${ autocomplete_base_url | n,unicode }"
  });

  huePubSub.subscribe('assist.mainObjectChange', function (db) {
    aceAutocomplete.setDatabase(db);
  });

  var options = ${ options_json | n,unicode };

  viewModel = new EditorViewModel(${ notebooks_json | n,unicode }, options);
  ko.applyBindings(viewModel);
  viewModel.init();

</script>

${ commonfooter(messages) | n,unicode }
