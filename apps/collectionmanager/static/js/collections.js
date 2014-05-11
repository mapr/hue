
// Defaults
var SOURCES = [
  'file',
  // 'hbase',
  // 'hive'
];

var SOURCE_TYPES = [
  'log',
  'separated',
  // 'morphlines'
];

var FIELD_TYPES = [
  'text_general',
  'string',
  'int',
  'float',
  'boolean',
  'date'
];

var FIELD_SEPARATORS = [
  ',',
  '\t',
];

var FIELD_QUOTE_CHARACTERS = [
  '"',
  "'"
];

// View Models

var HiveViewModel = function() {
  var self = this;

  self.databases = ko.observableArray();
  self.database = ko.observable().extend({'errors': null});
  self.table = ko.observable().extend({'errors': null});

  self.database.subscribe(function() {
    self.loadTables();
  });

  self.table.subscribe(function() {
    self.loadColumns();
  });

  self.loadDatabases = function() {
    return $.get("/beeswax/api/autocomplete").done(function(data) {
      if (data.databases.length > 0) {
        self.databases($.map(data.databases, function(database) { return new HiveDatabase(database); }));
        self.database(self.databases()[0]);
      }
    })
    .fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
    });
  };

  self.loadTables = function() {
    return $.get("/beeswax/api/autocomplete/" + self.database().name()).done(function(data) {
      if (data.tables.length > 0) {
        self.database().tables($.map(data.tables, function(table) { return new HiveTable(table); }));
        self.table(self.database().tables()[0]);
      }
    })
    .fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
    });
  };

  self.loadColumns = function() {
    return $.get("/beeswax/api/autocomplete/" + self.database().name() + '/' + self.table().name()).done(function(data) {
      if (data.columns.length > 0) {
        self.table().columns(data.columns);
      }
    })
    .fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
    });
  };

  // self.loadDatabases();
};

var HBaseViewModel = function() {
  var self = this;

  self.clusters = ko.observableArray();
  self.cluster = ko.observable().extend({'errors': null});
  self.table = ko.observable().extend({'errors': null});
  self.indexMapping = {};

  self.cluster.subscribe(function() {
    self.loadTables();
  });

  self.loadClusters = function() {
    return $.get("/hbase/api/getClusters").done(function(data) {
      if (data.data.length > 0) {
        self.clusters($.map(data.data, function(cluster) { return new HBaseCluster(cluster.name); }));
        self.cluster(self.clusters()[0]);
      }
    })
    .fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
    });
  };

  self.loadTables = function() {
    return $.get("/hbase/api/getTableList/" + self.cluster().name()).done(function(data) {
      if (data.data.length > 0) {
        // @TODO(Abe): Check if enabled
        self.cluster().tables($.map(data.data, function(table) { return new HBaseTable(table.name); }));
        self.table(self.cluster().tables()[0]);
      }
    })
    .fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
    });
  };

  // self.loadClusters();
};

var CreateCollectionViewModel = function() {
  var self = this;

  // Models
  self.collection = new Collection();

  self.source = ko.observable(SOURCES[0]).extend({'errors': null});
  self.fieldSeparator = ko.observable().extend({'errors': null});
  self.fieldQuoteCharacter = ko.observable().extend({'errors': null});
  self.file = ko.observable().extend({'errors': null});
  self.sourceType = ko.observable().extend({'errors': null});
  self.morphlines = ko.mapping.fromJS({'name': 'message', 'expression': '%{SYSLOGTIMESTAMP:timestamp} %{SYSLOGHOST:hostname} %{DATA:program}(?:\[%{POSINT:pid}\])?: %{GREEDYDATA:msg}'});
  self.morphlines.name = self.morphlines.name.extend({'errors': null});
  self.morphlines.expression = self.morphlines.expression.extend({'errors': null});

  // UI
  self.hive = new HiveViewModel();
  self.hbase = new HBaseViewModel();
  self.wizard = new Wizard();
  self.exampleSchema = ko.observable();
  self.isLoading = ko.observable();
  self.sources = ko.mapping.fromJS(SOURCES);
  self.fieldTypes = ko.mapping.fromJS(FIELD_TYPES);
  self.fieldSeparators = ko.mapping.fromJS(FIELD_SEPARATORS);
  self.fieldQuoteCharacters = ko.mapping.fromJS(FIELD_QUOTE_CHARACTERS);
  self.sourceTypes = ko.mapping.fromJS(SOURCE_TYPES);

  self.allCollectionChanges = ko.computed(function() {
    ko.toJS(self.collection.fields);

    if ('fetchExampleSchema' in self) {
      self.fetchExampleSchema();
    }
  });

  self.parseFields = function() {
    if (self.source() == 'file') {
      self.isLoading(true);
      return $.post("/collectionmanager/api/fields/parse/", {
        'source': self.source(),
        'type': self.sourceType(),
        'path': self.file(),
        'separator': self.fieldSeparator(),
        'quote': self.fieldQuoteCharacter(),
        'morphlines': ko.mapping.toJSON(self.morphlines)
      }).done(function(data) {
        if (data.status == 0) {
          self.collection.fields(inferFields(data.data, self.collection));

          // Find unique key default field
          var message = null;
          var first = null;
          ko.utils.arrayForEach(self.collection.fields(), function(field) {
            if (!first) {
              first = field;
            }
            if (field.name() == 'message') {
              message = field;
            }
          });
          if (message) {
            self.collection.uniqueKeyField(message.name());
          } else if (first) {
            self.collection.uniqueKeyField(first.name());
          }

          self.fetchExampleSchema();
        } else {
          $(document).trigger("error", data.message);
        }
        self.isLoading(false);
      }).fail(function (xhr, textStatus, errorThrown) {
        $(document).trigger("error", xhr.responseText);
        self.isLoading(false);
      });
    }
  };

  self.fetchExampleSchema = function() {
    self.isLoading(true);
    var collection = ko.mapping.toJS(self.collection);
    return $.post("/collectionmanager/api/schema/example/", {
      'collection': ko.mapping.toJSON(collection)
    }).done(function(data) {
      if (data.status == 0) {
        self.exampleSchema(data.example);
      } else {
        $(document).trigger("error", data.message);
      }
      self.isLoading(false);
    })
    .fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
      self.isLoading(false);
    });
  };

  self.save = function() {
    if (self.wizard.currentPage().validate()) {
      self.isLoading(true);
      var collection = ko.mapping.toJS(self.collection);
      collection.fields = ko.utils.arrayMap(collection.fields, function(field) {
        delete field['uniqueKeyField'];
        return field;
      });
      switch(self.source()) {
        case 'file':
        return $.post("/collectionmanager/api/collections/create/", {
          'collection': ko.mapping.toJSON(collection),
          'type': self.sourceType(),
          'path': self.file(),
          'source': self.source()
        }).done(function(data) {
          if (data.status == 0) {
            window.location.href = '/collectionmanager';
          } else {
            $(document).trigger("error", data.message);
          }
          self.isLoading(false);
        })
        .fail(function (xhr, textStatus, errorThrown) {
          $(document).trigger("error", xhr.responseText);
          self.isLoading(false);
        });

        case 'hive':
        return $.post("/collectionmanager/api/collections/create/", {
          'collection': ko.toJSON(collection),
          'database': self.hive.database().name(),
          'table': self.hive.table().name(),
          'source': self.source()
        }).done(function(data) {
          if (data.status == 0) {
            window.location.href = '/collectionmanager';
          } else {
            $(document).trigger("error", data.message);
          }
          self.isLoading(false);
        })
        .fail(function (xhr, textStatus, errorThrown) {
          $(document).trigger("error", xhr.responseText);
          self.isLoading(false);
        });
      }
    }
  };
};


var EditCollectionViewModel = function() {
  var self = this;

  // Models
  self.collection = ko.observable();
  self.sourceType = ko.observable().extend({'errors': null});

  // UI
  self.fieldTypes = ko.mapping.fromJS(FIELD_TYPES);
  self.sourceTypes = ko.mapping.fromJS(SOURCE_TYPES);
  self.isLoading = ko.observable();

  self.collection.subscribe(function(collection) {
    if (collection) {
      self.fetchFields();
    }
  });

  self.fetchFields = function() {
    if (self.collection()) {
      self.isLoading(true);
      return $.get("/collectionmanager/api/collections/" + self.collection().name() + "/metadata/").done(function(data) {
        if (data.status == 0) {
          self.collection().fields(inferFields(data.fields, self.collection()));
          self.collection().uniqueKeyField(data.unique_key);
          self.sourceType(data.type);
        } else {
          $(document).trigger("error", data.message);
        }
        self.isLoading(false);
      })
      .fail(function (xhr, textStatus, errorThrown) {
        $(document).trigger("error", xhr.responseText);
        self.isLoading(false);
      });
    }
  };

  self.updateCollection = function() {
    self.isLoading(true);
    var data = ko.mapping.toJS(self.collection);
    return $.post("/collectionmanager/api/collections/" + self.collection().name() + "/update/", {
      'collection': ko.mapping.toJSON(data),
      'type': self.sourceType()
    }).done(function(data) {
      if (data.status == 0) {
        $(document).trigger("info", data.message);
      } else {
        $(document).trigger("error", data.message);
      }
      self.isLoading(false);
    })
    .fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
      self.isLoading(false);
    });
  };
};


var ManageCollectionsViewModel = function() {
  var self = this;

  // Models
  self.collections = ko.observableArray();

  // UI
  self.isLoading = ko.observable();
  self.filteredCollections = ko.observableArray();
  self.selectedCollections = ko.computed(function() {
    return ko.utils.arrayFilter(self.collections(), function(collection) {
      return collection.selected();
    });
  });

  self.toggleCollectionSelect = function(collection, e) {
    ko.utils.arrayForEach(self.collections(), function(other_collection) {
      if(ko.unwrap(other_collection).name() == collection.name()) {
        other_collection.selected(!other_collection.selected());
      } else {
        if (!e.ctrlKey && !e.shiftKey) {
          other_collection.selected(false);
        }
      }
    });
  };

  self.importCollection = function(collection, e) {
    self.isLoading(true);
    return $.post("/collectionmanager/api/collections/import/", {
      'collection': ko.mapping.toJSON(collection)
    }).done(function(data) {
      self.isLoading(false);
      $(document).trigger("info", data.message);
      self.fetchCollections();
    })
    .fail(function (xhr, textStatus, errorThrown) {
      self.isLoading(false);
      $(document).trigger("error", xhr.responseText);
    });
  };

  self.filterTest = function(obj, filter) {
    return ko.unwrap(obj).name().indexOf(filter) != -1;
  };

  self.addData = function(collection, path) {
    self.isLoading(true);
    return $.post("/collectionmanager/api/collections/" + collection.name() + "/data/", {
      'collection': ko.mapping.toJSON(collection),
      'source': 'file',
      'path': path
    }).done(function(data) {
      self.isLoading(false);
      $(document).trigger("info", data.message);
    })
    .fail(function (xhr, textStatus, errorThrown) {
      self.isLoading(false);
      $(document).trigger("error", xhr.responseText);
    });
  };

  self.fetchCollections = function() {
    self.isLoading(true);
    return $.get("/collectionmanager/api/collections/").done(function(data) {
      if (data.status == 0) {
        var collections = [];
        ko.utils.arrayForEach(data.collections, function(collection) {
          var new_collection = ko.observable(new Collection(collection.name)).extend({'selectable': null});
          new_collection().hasHueCollection(collection.hue);
          new_collection().hasSolrCollection(collection.solr);
          collections.push(new_collection);
        });
        self.collections(collections);
      } else {
        $(document).trigger("error", data.message);
      }
      self.isLoading(false);
    })
    .fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
      self.isLoading(false);
    });
  };

  self.removeCollections = function(collection) {
    self.isLoading(true);
    var collections = self.filteredCollections();
    if (collections.length == 0) {
      collections = ($.isArray(collection)) ? collection : [collection];
    }
    return $.post("/collectionmanager/api/collections/remove/", {
      'collections': ko.mapping.toJSON(collections)
    }).done(function(data) {
      if (data.status == 0) {
        // Remove collections
        collection_names = ko.utils.arrayMap(collections, function(collection) {
          return ko.unwrap(collection).name();
        });
        var index = 0;
        var remove = [];
        ko.utils.arrayForEach(self.collections(), function(collection) {
          collection = ko.unwrap(collection);
          if (collection_names.indexOf(collection.name()) != -1) {
            remove.push(index);
          }
          index++;
        });
        remove.reverse();
        ko.utils.arrayForEach(remove, function(index) {
          self.collections.splice(index, 1);
        });
      } else {
        $(document).trigger("error", data.message);
      }
      self.isLoading(false);
    })
    .fail(function (xhr, textStatus, errorThrown) {
      $(document).trigger("error", xhr.responseText);
      self.isLoading(false);
    });
  };
};


var CollectionsViewModel = function() {
  var self = this;

  // Models
  self.page = ko.observable();

  // UI
  self.create = new CreateCollectionViewModel();
  self.manage = new ManageCollectionsViewModel();
  self.edit = new EditCollectionViewModel();
  self.isLoading = ko.computed(function() {
    return self.create.isLoading() || self.manage.isLoading() || self.edit.isLoading();
  });
};


// Utils

function getFileBrowseButton(inputElement) {
  return $("<button>").addClass("btn").addClass("fileChooserBtn").text("..").click(function (e) {
    e.preventDefault();
    $("#filechooser").jHueFileChooser({
      initialPath: inputElement.val(),
      onFileChoose: function (filePath) {
        inputElement.val(filePath);
        inputElement.trigger("change");
        $("#chooseFile").modal("hide");
      },
      selectFolder: false,
      createFolder: false
    });
    $("#chooseFile").modal("show");
  });
}


function inferFields(field_data, collection) {
  var fields = [];
  $.each(field_data, function(index, value) {
    // 0 => name
    // 1 => type
    // 2 => required
    // 3 => indexed
    // 4 => stored
    var field = new Field(collection, value[0], value[1], value[2], value[3]);
    field.saved(true);
    fields.push(field);
  });
  return fields;
}