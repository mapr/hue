// Licensed to Cloudera, Inc. under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Cloudera, Inc. licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

function magicLayout(vm) {
  loadLayout(vm, vm.initial.layout);
  $(document).trigger("magicLayout");
}

function loadLayout(viewModel, json_layout) {
  var _columns = [];

  $(json_layout).each(function (cnt, json_col) {
    var _rows = [];
    $(json_col.rows).each(function (rcnt, json_row) {
      var row = new Row([], viewModel);
      $(json_row.widgets).each(function (wcnt, widget) {
        row.addWidget(new Widget({
          size:widget.size,
          id: widget.id,
          name: widget.name,
          widgetType: widget.widgetType,
          properties: widget.properties,
          offset: widget.offset,
          loading: true,
          vm: viewModel
        }));
      });
      _rows.push(row);
    });
    var column = new Column(json_col.size, _rows);
    _columns = _columns.concat(column);
  });

  viewModel.columns(_columns);
}


// End dashboard lib

var Node = function (node) {
  var self = this;

  var type = typeof node.widgetType != "undefined" ? node.widgetType : node.type; 

  self.id = ko.observable(typeof node.id != "undefined" && node.id != null ? node.id : UUID());
  self.name = ko.observable(typeof node.name != "undefined" && node.name != null ? node.name : "");
  self.type = ko.observable(typeof type != "undefined" && type != null ? type : "");

  self.properties = ko.mapping.fromJS(typeof node.properties != "undefined" && node.properties != null ? node.properties : {});
  self.children = ko.mapping.fromJS(typeof node.children != "undefined" && node.children != null ? node.children : []);
}


var Workflow = function (vm, workflow) {
  var self = this;

  self.id = ko.observable(typeof workflow.id != "undefined" && workflow.id != null ? workflow.id : null);
  self.uuid = ko.observable(typeof workflow.uuid != "undefined" && workflow.uuid != null ? workflow.uuid : UUID());
  self.name = ko.observable(typeof workflow.name != "undefined" && workflow.name != null ? workflow.name : "");  

  self.properties = ko.mapping.fromJS(typeof workflow.properties != "undefined" && workflow.properties != null ? workflow.properties : {});
  self.nodes = ko.observableArray([]);
  
  self.loadNodes = function(workflow) {
    var nodes = []
    $.each(workflow.nodes, function(index, node) {
      var _node = new Node(node);
      nodes.push(_node);
    });
    self.nodes(nodes)
  }
  
  self.addNode = function(widget) {
    // Todo get parent cell, link nodes... when we have the new layout
    $.post("/oozie/editor/workflow/add_node/", {        
        "workflow": ko.mapping.toJSON(workflow),
        "node": ko.mapping.toJSON(widget),
        "properties": ko.mapping.toJSON(viewModel.addActionProperties()),        
      }, function (data) {
      if (data.status == 0) {
    	  widget.properties = data.properties;
          var node = new Node(ko.mapping.toJS(widget));
          node.children().push({'to': '33430f0f-ebfa-c3ec-f237-3e77efa03d0a'}) // Link to child

          // Add to list of nodes
          var end = self.nodes.pop()
          self.nodes.push(node);
          self.nodes.push(end);

          self.nodes()[0].children.removeAll(); // Parent link to new node
          self.nodes()[0].children().push({'to': node.id()});
      } else {
        $(document).trigger("error", data.message);
       }
     }).fail(function (xhr, textStatus, errorThrown) {
        $(document).trigger("error", xhr.responseText);
     });
  };
  
  self.getNodeById = function (node_id) {
    var _node = null;
    $.each(self.nodes(), function (index, node) {
      if (node.id() == node_id) {
        _node = node;
        return false;
      }
    });
    return _node;
  }
}

var WorkflowEditorViewModel = function (layout_json, workflow_json) {
  var self = this;

  self.isEditing = ko.observable(true);
  self.toggleEditing = function () {
    self.isEditing(! self.isEditing());
  };

  self.columns = ko.observable([]);
  self.previewColumns = ko.observable("");
  self.workflow = new Workflow(self, workflow_json);

  self.inited = ko.observable(self.columns().length > 0);
  self.init = function(callback) {
    loadLayout(self, layout_json);
    self.workflow.loadNodes(workflow_json);
  }

  self.addActionProperties = ko.observableArray([]);


  self.getWidgetById = function (widget_id) {
    var _widget = null;
    $.each(self.columns(), function (i, col) {
      $.each(col.rows(), function (j, row) {
        $.each(row.widgets(), function (z, widget) {
          if (widget.id() == widget_id){
            _widget = widget;
            return false;
          }
        });
      });
    });
    return _widget;
  }

  self.save = function () {
    $.post("/oozie/editor/workflow/save/", {        
        "layout": ko.mapping.toJSON(self.columns),
        "workflow": ko.mapping.toJSON(self.workflow)
    }, function (data) {
      if (data.status == 0) {
        self.workflow.id(data.id);
        $(document).trigger("info", data.message);
        if (window.location.search.indexOf("workflow") == -1) {
          window.location.hash = '#workflow=' + data.id;
        }
      }
      else {
        $(document).trigger("error", data.message);
     }
   }).fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
    });
  };

  self.gen_xml = function () {
    $.post("/oozie/editor/workflow/gen_xml/", {        
        "layout": ko.mapping.toJSON(self.columns),
        "workflow": ko.mapping.toJSON(self.workflow)
    }, function (data) {
      if (data.status == 0) {
        alert(data.xml);
      }
      else {
        $(document).trigger("error", data.message);
     }
   }).fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
    });
  };

  self.showSubmitPopup = function () {
    // if self.workflow.id() == null, need to save wf for now

    $.get("/oozie/editor/workflow/submit/" + self.workflow.id(), {
      }, function (data) {
        $(document).trigger("showSubmitPopup", data);
     }).fail(function (xhr, textStatus, errorThrown) {
        $(document).trigger("error", xhr.responseText);
      });
    };

    
  function bareWidgetBuilder(name, type){
    return new Widget({
      size: 12,
      id: UUID(),
      name: name,
      widgetType: type
    });
  }

  self.draggableHiveAction = ko.observable(bareWidgetBuilder("Hive Script", "hive-widget"));
  self.draggablePigAction = ko.observable(bareWidgetBuilder("Pig Script", "pig-widget"));
  self.draggableJavaAction = ko.observable(bareWidgetBuilder("Java program", "java-widget"));
  self.draggableMapReduceAction = ko.observable(bareWidgetBuilder("MapReduce job", "mapreduce-widget"));
};
