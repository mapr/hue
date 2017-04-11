# encoding: utf-8
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

from south.v2 import DataMigration

import desktop.management.commands.convert_documents

class Migration(DataMigration):

    def forwards(self, orm):
        # Earlier we did the document conversions from Doc1 to Doc2 upon loading
        # the home page of a user. That approach had certain flaws like shared
        # documents didn't show up until the owner logged in and opened his home
        # page. Also, home page load time was affected when the conversions failed
        # and loading the home page retried the conversions every single time.
        # This migration handles the document conversion of all users at
        # the same time preventing such flaws.

        desktop.management.commands.convert_documents.Command().handle_noargs()

    def backwards(self, orm):
        pass

    models = {
        u'auth.group': {
            'Meta': {'object_name': 'Group'},
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'unique': 'True', 'max_length': '80'}),
            'permissions': ('django.db.models.fields.related.ManyToManyField', [], {'to': u"orm['auth.Permission']", 'symmetrical': 'False', 'blank': 'True'})
        },
        u'auth.permission': {
            'Meta': {'ordering': "(u'content_type__app_label', u'content_type__model', u'codename')", 'unique_together': "((u'content_type', u'codename'),)", 'object_name': 'Permission'},
            'codename': ('django.db.models.fields.CharField', [], {'max_length': '100'}),
            'content_type': ('django.db.models.fields.related.ForeignKey', [], {'to': u"orm['contenttypes.ContentType']"}),
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'max_length': '50'})
        },
        u'auth.user': {
            'Meta': {'object_name': 'User'},
            'date_joined': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now'}),
            'email': ('django.db.models.fields.EmailField', [], {'max_length': '75', 'blank': 'True'}),
            'first_name': ('django.db.models.fields.CharField', [], {'max_length': '30', 'blank': 'True'}),
            'groups': ('django.db.models.fields.related.ManyToManyField', [], {'symmetrical': 'False', 'related_name': "u'user_set'", 'blank': 'True', 'to': u"orm['auth.Group']"}),
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'is_active': ('django.db.models.fields.BooleanField', [], {'default': 'True'}),
            'is_staff': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'is_superuser': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'last_login': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now'}),
            'last_name': ('django.db.models.fields.CharField', [], {'max_length': '30', 'blank': 'True'}),
            'password': ('django.db.models.fields.CharField', [], {'max_length': '128'}),
            'user_permissions': ('django.db.models.fields.related.ManyToManyField', [], {'symmetrical': 'False', 'related_name': "u'user_set'", 'blank': 'True', 'to': u"orm['auth.Permission']"}),
            'username': ('django.db.models.fields.CharField', [], {'unique': 'True', 'max_length': '30'})
        },
        u'contenttypes.contenttype': {
            'Meta': {'ordering': "('name',)", 'unique_together': "(('app_label', 'model'),)", 'object_name': 'ContentType', 'db_table': "'django_content_type'"},
            'app_label': ('django.db.models.fields.CharField', [], {'max_length': '100'}),
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'model': ('django.db.models.fields.CharField', [], {'max_length': '100'}),
            'name': ('django.db.models.fields.CharField', [], {'max_length': '100'})
        },
        u'desktop.defaultconfiguration': {
            'Meta': {'ordering': "['app', '-is_default', 'user']", 'object_name': 'DefaultConfiguration'},
            'app': ('django.db.models.fields.CharField', [], {'max_length': '32', 'db_index': 'True'}),
            'groups': ('django.db.models.fields.related.ManyToManyField', [], {'db_index': 'True', 'to': u"orm['auth.Group']", 'db_table': "'defaultconfiguration_groups'", 'symmetrical': 'False'}),
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'is_default': ('django.db.models.fields.BooleanField', [], {'default': 'False', 'db_index': 'True'}),
            'properties': ('django.db.models.fields.TextField', [], {'default': "'[]'"}),
            'user': ('django.db.models.fields.related.ForeignKey', [], {'to': u"orm['auth.User']", 'null': 'True', 'blank': 'True'})
        },
        u'desktop.document': {
            'Meta': {'unique_together': "(('content_type', 'object_id'),)", 'object_name': 'Document'},
            'content_type': ('django.db.models.fields.related.ForeignKey', [], {'to': u"orm['contenttypes.ContentType']"}),
            'description': ('django.db.models.fields.TextField', [], {'default': "''"}),
            'extra': ('django.db.models.fields.TextField', [], {'default': "''"}),
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'last_modified': ('django.db.models.fields.DateTimeField', [], {'auto_now': 'True', 'db_index': 'True', 'blank': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'default': "''", 'max_length': '255'}),
            'object_id': ('django.db.models.fields.PositiveIntegerField', [], {}),
            'owner': ('django.db.models.fields.related.ForeignKey', [], {'related_name': "'doc_owner'", 'to': u"orm['auth.User']"}),
            'tags': ('django.db.models.fields.related.ManyToManyField', [], {'to': u"orm['desktop.DocumentTag']", 'db_index': 'True', 'symmetrical': 'False'}),
            'version': ('django.db.models.fields.SmallIntegerField', [], {'default': '1'})
        },
        u'desktop.document2': {
            'Meta': {'ordering': "['-last_modified', 'name']", 'unique_together': "(('uuid', 'version', 'is_history'),)", 'object_name': 'Document2'},
            'data': ('django.db.models.fields.TextField', [], {'default': "'{}'"}),
            'dependencies': ('django.db.models.fields.related.ManyToManyField', [], {'db_index': 'True', 'related_name': "'dependents'", 'symmetrical': 'False', 'to': u"orm['desktop.Document2']"}),
            'description': ('django.db.models.fields.TextField', [], {'default': "''"}),
            'extra': ('django.db.models.fields.TextField', [], {'default': "''"}),
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'is_history': ('django.db.models.fields.BooleanField', [], {'default': 'False', 'db_index': 'True'}),
            'is_managed': ('django.db.models.fields.BooleanField', [], {'default': 'False', 'db_index': 'True'}),
            'is_trashed': ('django.db.models.fields.NullBooleanField', [], {'default': 'False', 'null': 'True', 'db_index': 'True', 'blank': 'True'}),
            'last_modified': ('django.db.models.fields.DateTimeField', [], {'auto_now': 'True', 'db_index': 'True', 'blank': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'default': "''", 'max_length': '255'}),
            'owner': ('django.db.models.fields.related.ForeignKey', [], {'related_name': "'doc2_owner'", 'to': u"orm['auth.User']"}),
            'parent_directory': ('django.db.models.fields.related.ForeignKey', [], {'blank': 'True', 'related_name': "'children'", 'null': 'True', 'to': u"orm['desktop.Document2']"}),
            'search': ('django.db.models.fields.TextField', [], {'null': 'True', 'blank': 'True'}),
            'type': ('django.db.models.fields.CharField', [], {'default': "''", 'max_length': '32', 'db_index': 'True'}),
            'uuid': ('django.db.models.fields.CharField', [], {'default': "'7127eba0-4091-4235-9321-f692d22a965b'", 'max_length': '36', 'db_index': 'True'}),
            'version': ('django.db.models.fields.SmallIntegerField', [], {'default': '1', 'db_index': 'True'})
        },
        u'desktop.document2permission': {
            'Meta': {'unique_together': "(('doc', 'perms'),)", 'object_name': 'Document2Permission'},
            'doc': ('django.db.models.fields.related.ForeignKey', [], {'to': u"orm['desktop.Document2']"}),
            'groups': ('django.db.models.fields.related.ManyToManyField', [], {'db_index': 'True', 'to': u"orm['auth.Group']", 'db_table': "'documentpermission2_groups'", 'symmetrical': 'False'}),
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'perms': ('django.db.models.fields.CharField', [], {'default': "'read'", 'max_length': '10', 'db_index': 'True'}),
            'users': ('django.db.models.fields.related.ManyToManyField', [], {'db_index': 'True', 'to': u"orm['auth.User']", 'db_table': "'documentpermission2_users'", 'symmetrical': 'False'})
        },
        u'desktop.documentpermission': {
            'Meta': {'unique_together': "(('doc', 'perms'),)", 'object_name': 'DocumentPermission'},
            'doc': ('django.db.models.fields.related.ForeignKey', [], {'to': u"orm['desktop.Document']"}),
            'groups': ('django.db.models.fields.related.ManyToManyField', [], {'db_index': 'True', 'to': u"orm['auth.Group']", 'db_table': "'documentpermission_groups'", 'symmetrical': 'False'}),
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'perms': ('django.db.models.fields.CharField', [], {'default': "'read'", 'max_length': '10'}),
            'users': ('django.db.models.fields.related.ManyToManyField', [], {'db_index': 'True', 'to': u"orm['auth.User']", 'db_table': "'documentpermission_users'", 'symmetrical': 'False'})
        },
        u'desktop.documenttag': {
            'Meta': {'unique_together': "(('owner', 'tag'),)", 'object_name': 'DocumentTag'},
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'owner': ('django.db.models.fields.related.ForeignKey', [], {'to': u"orm['auth.User']"}),
            'tag': ('django.db.models.fields.SlugField', [], {'max_length': '50'})
        },
        u'desktop.settings': {
            'Meta': {'object_name': 'Settings'},
            'collect_usage': ('django.db.models.fields.BooleanField', [], {'default': 'True', 'db_index': 'True'}),
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'tours_and_tutorials': ('django.db.models.fields.BooleanField', [], {'default': 'True', 'db_index': 'True'})
        },
        u'desktop.userpreferences': {
            'Meta': {'object_name': 'UserPreferences'},
            u'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'key': ('django.db.models.fields.CharField', [], {'max_length': '20'}),
            'user': ('django.db.models.fields.related.ForeignKey', [], {'to': u"orm['auth.User']"}),
            'value': ('django.db.models.fields.TextField', [], {'max_length': '4096'})
        }
    }

    complete_apps = ['desktop']
    symmetrical = True
