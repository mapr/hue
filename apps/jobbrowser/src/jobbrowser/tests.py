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


import json
import logging
import time
import unittest
import re
import hadoop
import hadoop.yarn.resource_manager_api as resource_manager_api
from hadoop.yarn import mapreduce_api
import desktop.lib.mapr_test_utils as mapr_test_utils
from hadoop import cluster
from django.contrib.auth.models import User
from django.core.urlresolvers import reverse
from nose.plugins.skip import SkipTest
from nose.tools import assert_true, assert_false, assert_equal

from desktop.lib.django_test_util import make_logged_in_client
from desktop.lib import thrift_util
from desktop.lib.test_utils import grant_access, add_to_group
from desktop.models import Document
from hadoop import cluster
from hadoop.conf import YARN_CLUSTERS
from hadoop.yarn import resource_manager_api, mapreduce_api, history_server_api
from liboozie.oozie_api_tests import OozieServerProvider
from oozie.models import Workflow

from jobbrowser import models, views
from jobbrowser.conf import SHARE_JOBS
from jobbrowser.models import can_view_job, can_modify_job, Job, LinkJobLogs
from hadoop import conf as webhdfs_conf
from hadoop.fs.webhdfs import WebHdfs


LOG = logging.getLogger(__name__)
_INITIALIZED = False


class TestBrowser():

  def test_dots_to_camel_case(self):
    assert_equal("fooBar", models.dots_to_camel_case("foo.bar"))
    assert_equal("fooBarBaz", models.dots_to_camel_case("foo.bar.baz"))
    assert_equal("foo", models.dots_to_camel_case("foo"))
    assert_equal("foo.", models.dots_to_camel_case("foo."))

  def test_get_path(self):
    assert_equal("/foo/bar", models.get_path("hdfs://host/foo/bar"))

  def test_format_counter_name(self):
    assert_equal("Foo Bar", views.format_counter_name("fooBar"))
    assert_equal("Foo Bar Baz", views.format_counter_name("fooBarBaz"))
    assert_equal("Foo", views.format_counter_name("foo"))
    assert_equal("Foo.", views.format_counter_name("foo."))
    assert_equal("A Bbb Ccc", views.format_counter_name("A_BBB_CCC"))

def get_hadoop_job_id(oozie_api, oozie_jobid, action_index=1, timeout=60, step=5):
  hadoop_job_id = None
  start = time.time()
  while not hadoop_job_id and time.time() - start < timeout:
    time.sleep(step)
    hadoop_job_id = oozie_api.get_job(oozie_jobid).actions[action_index].externalId
  if not hadoop_job_id:
    logs = OozieServerProvider.oozie.get_job_log(oozie_jobid)
    msg = "[%d] %s took more than %d to create a job: %s" % (time.time(), oozie_jobid, timeout, logs)
    LOG.info(msg)
    raise Exception(msg)
  return hadoop_job_id

class TestJobBrowserSecurity(unittest.TestCase, OozieServerProvider):

    requires_hadoop = True

    @classmethod
    def setup_class(cls):
        OozieServerProvider.setup_class()
        cluster_conf = webhdfs_conf.HDFS_CLUSTERS['default']
        cls.fs = WebHdfs.from_config(cluster_conf)
        cls.username = 'mapr'
        cls.client = make_logged_in_client(username=cls.username, is_superuser=False, groupname='test')
        grant_access(cls.username, 'test', 'jobsub')
        grant_access(cls.username, 'test', 'jobbrowser')
        grant_access(cls.username, 'test', 'oozie')
        add_to_group(cls.username)

    def test_maprsasl_mechanism(self):
        cluster_conf = hadoop.cluster.get_cluster_conf_for_job_submission()
        if cluster_conf.MECHANISM.get() != 'MAPR-SECURITY':
            raise SkipTest
        if webhdfs_conf.MR_CLUSTERS['default'].SUBMIT_TO.get():
            self.check_security(webhdfs_conf.MR_CLUSTERS['default'], 'Could not start SASL: Error in sasl_client_start', 'MAPR-SECURITY')
        else:
            self.check_security(YARN_CLUSTERS['default'], 'Unable to authenticate <Response [401]>', 'MAPR-SECURITY')

    def check_security(self, rm_jt_conf, message, security_type):
        try:
            self.client.get('/jobbrowser/jobs/?format=json')
            current_mechanism = rm_jt_conf.MECHANISM.get()
            rm_jt_conf.MECHANISM.set_for_testing('none')
            self.clean_cache()
            try:
                self.client.get('/jobbrowser/jobs/?format=json')
            except Exception as e:
                assert_true(message in str(e), "Should be failed because of incorrect security mechanism")
            else:
                raise AssertionError('request should fail because of wrong security mechanism')
            rm_jt_conf.MECHANISM.set_for_testing(security_type)
            self.clean_cache()
            response = self.client.get('/jobbrowser/jobs/?format=json')
            assert_equal(response.status_code, 200, "Incorrect response status %s. Should be 200 OK"
                         % response.status_code)
        finally:
            # clean up test results
            self.clean_cache()
            rm_jt_conf.MECHANISM.set_for_testing(current_mechanism)

    def clean_cache(self):
        thrift_util._connection_pool = thrift_util.ConnectionPooler()
        resource_manager_api._api_cache = None
        cluster.MR_CACHE = None
        cluster.get_default_mrcluster()

    def test_kerberos_mechanism(self):
        cluster_conf = hadoop.cluster.get_cluster_conf_for_job_submission()
        if cluster_conf.MECHANISM.get() != 'GSSAPI':
            raise SkipTest
        if webhdfs_conf.MR_CLUSTERS['default'].SUBMIT_TO.get():
            self.check_security(webhdfs_conf.MR_CLUSTERS['default'], 'Could not start SASL: Error in sasl_client_start',
                                'GSSAPI')
        else:
            self.check_security(YARN_CLUSTERS['default'], 'Unable to authenticate <Response [401]>', 'GSSAPI')

class TestJobBrowserWithHadoop(unittest.TestCase, OozieServerProvider):

  requires_hadoop = True

  @classmethod
  def setup_class(cls):
    OozieServerProvider.setup_class()
    cluster_conf = webhdfs_conf.HDFS_CLUSTERS['default']
    cls.fs = WebHdfs.from_config(cluster_conf)
    cls.username = 'mapr'
    cls.home_dir = '/user/%s' % cls.username
    cls.fs.do_as_user(cls.username, cls.fs.create_home_dir, cls.home_dir)

    cls.client = make_logged_in_client(username=cls.username, is_superuser=False, groupname='test')
    cls.user = User.objects.get(username=cls.username)
    grant_access(cls.username, 'test', 'jobsub')
    grant_access(cls.username, 'test', 'jobbrowser')
    grant_access(cls.username, 'test', 'oozie')
    add_to_group(cls.username)

    cls.prev_user = cls.fs.user

    cls.install_examples()
    cls.design = cls.create_design()

    # Run the sleep example, since it doesn't require user home directory
    design_id = cls.design.id
    response = cls.client.post(reverse('oozie:submit_workflow',
                                args=[design_id]),
                                data={u'form-MAX_NUM_FORMS': [u''],
                                      u'form-INITIAL_FORMS': [u'1'],
                                      u'form-0-name': [u'REDUCER_SLEEP_TIME'],
                                      u'form-0-value': [u'1'],
                                      u'form-TOTAL_FORMS': [u'1']},
                                follow=True)
    oozie_jobid = response.context['oozie_workflow'].id
    OozieServerProvider.wait_until_completion(oozie_jobid)

    cls.hadoop_job_id = get_hadoop_job_id(cls.oozie, oozie_jobid, 1)
    cls.hadoop_job_id_short = views.get_shorter_id(cls.hadoop_job_id)

  @classmethod
  def teardown_class(cls):
    try:
      Document.objects.filter(name__contains=cls.username).delete()
      Workflow.objects.filter(name__contains=cls.username).delete()
      # Remove user home directories.
      cls.fs.do_as_superuser(cls.fs.rmtree, cls.home_dir)
    except:
      LOG.exception('failed to teardown %s' % cls.home_dir)
    cls.fs.setuser(cls.prev_user)

  @classmethod
  def create_design(cls):
    job_name = '%s_%s' % (cls.username, 'sleep_job')
    if not Document.objects.available_docs(Workflow, cls.user).filter(name=job_name).exists():
      response = cls.client.post(reverse('jobsub.views.new_design',
        kwargs={'node_type': 'mapreduce'}),
        data={'name': job_name,
              'description': '',
              'node_type': 'mapreduce',
              'jar_path': '/oozie/workspaces/lib/hadoop-examples.jar',
              'prepares': '[]',
              'files': '[]',
              'archives': '[]',
              'job_properties': '[{\"name\":\"mapred.reduce.tasks\",\"value\":\"1\"},{\"name\":\"mapred.mapper.class\",\"value\":\"org.apache.hadoop.examples.SleepJob\"},{\"name\":\"mapred.reducer.class\",\"value\":\"org.apache.hadoop.examples.SleepJob\"},{\"name\":\"mapred.mapoutput.key.class\",\"value\":\"org.apache.hadoop.io.IntWritable\"},{\"name\":\"mapred.mapoutput.value.class\",\"value\":\"org.apache.hadoop.io.NullWritable\"},{\"name\":\"mapred.output.format.class\",\"value\":\"org.apache.hadoop.mapred.lib.NullOutputFormat\"},{\"name\":\"mapred.input.format.class\",\"value\":\"org.apache.hadoop.examples.SleepJob$SleepInputFormat\"},{\"name\":\"mapred.partitioner.class\",\"value\":\"org.apache.hadoop.examples.SleepJob\"},{\"name\":\"mapred.speculative.execution\",\"value\":\"false\"},{\"name\":\"sleep.job.map.sleep.time\",\"value\":\"0\"},{\"name\":\"sleep.job.reduce.sleep.time\",\"value\":\"${REDUCER_SLEEP_TIME}\"}]'
        },
        HTTP_X_REQUESTED_WITH='XMLHttpRequest')
      assert_equal(response.status_code, 200, "Incorrect status code %s, should be 200" % response.status_code)

    return Document.objects.available_docs(Workflow, cls.user).get(name=job_name).content_object

  @classmethod
  def install_examples(cls):
    global _INITIALIZED
    if _INITIALIZED:
      return

    cls.client.post(reverse('oozie:install_examples'))
    cls.fs.do_as_user(cls.username, cls.fs.create_home_dir, cls.home_dir)
    # cls.fs.do_as_superuser(cls.fs.chmod, cls.home_dir, 0777, True)

    _INITIALIZED = True

  def execute_sleep_job(self):
    response = self.client.post(reverse('oozie:submit_workflow',
                                args=[self.design.id]),
                                data={u'form-MAX_NUM_FORMS': [u''],
                                      u'form-INITIAL_FORMS': [u'1'],
                                      u'form-0-name': [u'REDUCER_SLEEP_TIME'],
                                      u'form-0-value': [u'1'],
                                      u'form-TOTAL_FORMS': [u'1']},
                                follow=True)
    oozie_jobid = response.context['oozie_workflow'].id
    OozieServerProvider.wait_until_completion(oozie_jobid)

    self.hadoop_job_id = get_hadoop_job_id(self.oozie, oozie_jobid, 1)
    self.hadoop_job_id_short = views.get_shorter_id(self.hadoop_job_id)

  def test_uncommon_views(self):
    """
    These views exist, but tend not to be ever called, because they're not in the normal UI.
    """
    raise SkipTest
    TestJobBrowserWithHadoop.client.get("/jobbrowser/clusterstatus")
    TestJobBrowserWithHadoop.client.get("/jobbrowser/queues")
    TestJobBrowserWithHadoop.client.get("/jobbrowser/jobbrowser")

  def test_failed_jobs(self):
    """
    Test jobs with genuine failure, not just killed
    """
    # Create design that will fail because the script file isn't there
    INPUT_DIR = TestJobBrowserWithHadoop.home_dir + '/input'
    OUTPUT_DIR = TestJobBrowserWithHadoop.home_dir + '/output'
    try:
      TestJobBrowserWithHadoop.fs.mkdir(TestJobBrowserWithHadoop.home_dir + "/jt-test_failed_jobs")
      TestJobBrowserWithHadoop.fs.mkdir(INPUT_DIR)
      TestJobBrowserWithHadoop.fs.rmtree(OUTPUT_DIR)
    except:
      LOG.exception('failed to teardown tests')

    job_name = '%s_%s' % (TestJobBrowserWithHadoop.username, 'test_failed_jobs-1')
    response = TestJobBrowserWithHadoop.client.post(reverse('jobsub.views.new_design', kwargs={'node_type': 'mapreduce'}), {
        'name': [job_name],
        'description': ['description test_failed_jobs-1'],
        'args': '',
        'jar_path': '/oozie/workspaces/lib/hadoop-examples.jar',
        'prepares': '[]',
        'archives': '[]',
        'files': '[]',
        'job_properties': ['[{"name":"mapred.input.dir","value":"%s"},\
            {"name":"mapred.output.dir","value":"%s"},\
            {"name":"mapred.mapper.class","value":"org.apache.hadoop.mapred.lib.dne"},\
            {"name":"mapred.combiner.class","value":"org.apache.hadoop.mapred.lib.dne"},\
            {"name":"mapred.reducer.class","value":"org.apache.hadoop.mapred.lib.dne"}]' % (INPUT_DIR, OUTPUT_DIR)]
        }, HTTP_X_REQUESTED_WITH='XMLHttpRequest', follow=True)

    # Submit the job
    design_dict = json.loads(response.content)
    design_id = int(design_dict['id'])
    response = TestJobBrowserWithHadoop.client.post(reverse('oozie:submit_workflow',
                                args=[design_id]),
                                data={u'form-MAX_NUM_FORMS': [u''],
                                      u'form-INITIAL_FORMS': [u'1'],
                                      u'form-0-name': [u'REDUCER_SLEEP_TIME'],
                                      u'form-0-value': [u'1'],
                                      u'form-TOTAL_FORMS': [u'1']},
                                follow=True)
    oozie_jobid = response.context['oozie_workflow'].id
    job = OozieServerProvider.wait_until_completion(oozie_jobid)
    hadoop_job_id = get_hadoop_job_id(TestJobBrowserWithHadoop.oozie, oozie_jobid, 1)
    hadoop_job_id_short = views.get_shorter_id(hadoop_job_id)

    # Select only killed jobs (should be absent)
    # Taking advantage of the fact new jobs are at the top of the list!
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/?format=json&state=killed')
    assert_false(hadoop_job_id_short in response.content, "No id in response")

    # Select only failed jobs (should be present)
    # Map job should succeed. Reduce job should fail.
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/?format=json&state=failed')
    assert_true(hadoop_job_id_short in response.content, "No id in response")

    # raise SkipTest # Not compatible with MR2

    if webhdfs_conf.MR_CLUSTERS['default'].SUBMIT_TO.get():
        raise SkipTest
    # The single job view should have the failed task table
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s' % (hadoop_job_id,))
    html = response.content.lower()
    assert_true('failed task' in html, 'Failed task not in the response')

    # get map id
    map_task_id = str(int(hadoop_job_id[-4:]) - 1)
    while len(map_task_id) < 4:
        map_task_id = '0' + map_task_id
    map_hadoop_job_id = hadoop_job_id[:-4] + map_task_id
    map_task_id = (map_hadoop_job_id + '_m_000000').replace('job', 'task')
    # The map task should say success (empty input)
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks/%s' % (map_hadoop_job_id, map_task_id))
    assert_true('succeed' in response.content)
    assert_true('failed' not in response.content)

    # The reduce task should say failed
    reduce_task_id = hadoop_job_id.replace('job', 'task') + '_r_000000'
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks/%s' % (hadoop_job_id, reduce_task_id))
    assert_true('succeed' not in response.content, 'Succeed not in the response')
    assert_true('failed' in response.content, 'Failed not in the response')

    # Selecting by failed state should include the failed map
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks?taskstate=failed' % (hadoop_job_id,))
    assert_true('r_000000' in response.content, 'Missing reduce task')
    assert_true('m_000000' not in response.content, 'Missing map task')

  def test_jobs_page(self):
    # All jobs page and fetch job ID
    # Taking advantage of the fact new jobs are at the top of the list!
    if webhdfs_conf.MR_CLUSTERS['default'].SUBMIT_TO.get():
        self.execute_sleep_job()
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s' % (self.hadoop_job_id,))
    assert_true(self.hadoop_job_id_short in response.content, 'No id in response')
    time.sleep(2)
    # Make sure job succeeded
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/?format=json&state=completed', kwargs={'retired':True})
    assert_true(self.hadoop_job_id_short in response.content, 'Not in completed jobs')
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/?format=json&state=failed')
    assert_false(self.hadoop_job_id_short in response.content, 'Job is in failed jobs')
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/?format=json&state=running')
    assert_false(self.hadoop_job_id_short in response.content, 'Job is in rinning jobs')
    response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/?format=json&state=killed')
    assert_false(self.hadoop_job_id_short in response.content, 'Job is in killed jobs')

  def start_sleep_job(self):
      response = self.client.post(reverse('oozie:submit_workflow',
                                          args=[self.design.id]),
                                  data={u'form-MAX_NUM_FORMS': [u''],
                                        u'form-INITIAL_FORMS': [u'1'],
                                        u'form-0-name': [u'REDUCER_SLEEP_TIME'],
                                        u'form-0-value': [u'1'],
                                        u'form-TOTAL_FORMS': [u'1']},
                                  follow=True)
      # need a little delay before job will appear in JT and we can get it's id
      oozie_jobid = response.context['oozie_workflow'].id
      self.hadoop_job_id = get_hadoop_job_id(self.oozie, oozie_jobid, 1)
      self.hadoop_job_id_short = views.get_shorter_id(self.hadoop_job_id)
      return oozie_jobid

  def test_tasks_page(self):
    # Test tracker page
    if webhdfs_conf.MR_CLUSTERS['default'].SUBMIT_TO.get():
        oozie_jobid = self.start_sleep_job()
        early_task_id = str(self.hadoop_job_id.replace('job', 'task')) + '_m_000000'
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks/%s' % (self.hadoop_job_id, early_task_id))
        assert_true(early_task_id in response.content, "Cant get task with id %s" % str(early_task_id))
        assert_true('running' in response.content, "Task is not in running status")
        assert_true('<td>MAP</td>' in response.content, "Task type is not map")
        # wait while task will get successful status
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks/%s' % (self.hadoop_job_id, early_task_id))
        start_time = time.time()
        while time.time() - start_time < 60:
            if 'succeeded' in response.content:
                break
            else:
                response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks/%s' % (self.hadoop_job_id, early_task_id))
        else:
            raise AssertionError("Task didn't reach succeeded status")
        OozieServerProvider.wait_until_completion(oozie_jobid)
    else:
        early_task_id = str(self.hadoop_job_id.replace('job', 'task')) + '_m_000000'
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks/%s' % (self.hadoop_job_id, early_task_id))
        assert_true('Job Task: ' + early_task_id in response.content, 'Job Task is not on the page')
        assert_true('succeeded' in response.content, 'succeeded is not on the page')

  def test_job_permissions(self):
    if webhdfs_conf.MR_CLUSTERS['default'].SUBMIT_TO.get():
        raise SkipTest
    # Login as ourself
    finish = SHARE_JOBS.set_for_testing(True)
    try:
      response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/?format=json&user=')
      assert_true(TestJobBrowserWithHadoop.hadoop_job_id_short in response.content, 'Missing job id in response')
    finally:
      finish()

    finish = SHARE_JOBS.set_for_testing(False)
    try:
      response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/?format=json&user=')
      assert_true(TestJobBrowserWithHadoop.hadoop_job_id_short in response.content, 'Missing job id in response')
    finally:
      finish()
    mapr_test_utils.create_user('not_me')
    # Login as someone else
    client_not_me = make_logged_in_client(username='not_me', password='not_me', is_superuser=False, groupname='test')
    grant_access("not_me", "test", "jobbrowser")

    finish = SHARE_JOBS.set_for_testing(True)
    try:
      response = client_not_me.get('/jobbrowser/jobs/?format=json&user=')
      assert_true(TestJobBrowserWithHadoop.hadoop_job_id_short in response.content, 'Missing job id in response')
    finally:
      finish()

    finish = SHARE_JOBS.set_for_testing(False)
    try:
      response = client_not_me.get('/jobbrowser/jobs/?format=json&user=')
      assert_false(TestJobBrowserWithHadoop.hadoop_job_id_short in response.content, 'Missing job id in response')
    finally:
      finish()

  def test_job_counter(self):
    if YARN_CLUSTERS['default'].SUBMIT_TO.get():
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s' % TestJobBrowserWithHadoop.hadoop_job_id)
        # Check some counters for single job.
        counters = response.context['job'].counters
        assert_true('org.apache.hadoop.mapreduce.FileSystemCounter'
                    in counters['counterGroup'][0]['counterGroupName'], 'FileSystemCounter is not in counter')
        assert_true('org.apache.hadoop.mapreduce.JobCounter'
                    in counters['counterGroup'][1]['counterGroupName'], 'JobCounter is not in counter')
        assert_true('org.apache.hadoop.mapreduce.TaskCounter'
                    in counters['counterGroup'][2]['counterGroupName'], 'TaskCounter is not in counter')
        assert_true('Shuffle Errors'
                    in counters['counterGroup'][3]['counterGroupName'], 'Shuffle Errors is not in counter')
        assert_true('org.apache.hadoop.mapreduce.lib.input.FileInputFormatCounter'
                    in counters['counterGroup'][4]['counterGroupName'], 'FileInputFormatCounter is not in counter')
        assert_true('org.apache.hadoop.mapreduce.lib.output.FileOutputFormatCounter'
                    in counters['counterGroup'][5]['counterGroupName'], 'FileOutputFormatCounter is not in counter')
    else:
        self.execute_sleep_job()
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s' % self.hadoop_job_id)
        # Check some counters for single job.
        counters = response.context['job'].counters
        counters_file_bytes_written = counters['org.apache.hadoop.mapreduce.FileSystemCounter']['counters']['FILE_BYTES_WRITTEN']
        assert_true(counters_file_bytes_written['map'] > 0, 'Map counters should be > 0')
        assert_true(counters_file_bytes_written['reduce'] > 0, 'Reduce counters should be > 0')

  def check_task_property(self, task_prop, expected_value, error_message):
      start_time = time.time()
      response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks?%s=%s' % (self.hadoop_job_id, task_prop,
                                                                                          expected_value))
      while time.time() - start_time < 60:
          if expected_value == response.context[0]._data[task_prop]:
              break
          else:
              response = TestJobBrowserWithHadoop.client.get(
                  '/jobbrowser/jobs/%s/tasks?%s=%s' % (self.hadoop_job_id, task_prop,
                                                                expected_value))
      else:
          raise AssertionError(error_message)

  def test_task_page(self):
    if webhdfs_conf.MR_CLUSTERS['default'].SUBMIT_TO.get():
        self.start_sleep_job()
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks?tasktype=reduce' % (self.hadoop_job_id,))
        assert_equal(response.context[0]._data['tasktype'], 'reduce', 'Task type is not reduce')
        # wait until succeeded state
        self.check_task_property("taskstate", "succeeded", "Task didn't reach succeeded state")
        # wait until clean state
        self.check_task_property("tasktext", "clean", "Task tasktext is not cleaned")
    else:
        # Select by tasktype
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks?tasktype=reduce' % (TestJobBrowserWithHadoop.hadoop_job_id,))
        assert_equal(response.context[0]._data['tasktype'], 'reduce', 'Task type is not reduce')
        # Select by taskstate
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks?taskstate=succeeded' % (TestJobBrowserWithHadoop.hadoop_job_id,))
        assert_equal(response.context[0]._data['taskstate'], 'succeeded', 'Task state is not succeeded')
        # Select by text
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/tasks?tasktext=clean' % (TestJobBrowserWithHadoop.hadoop_job_id,))
        assert_equal(response.context[0]._data['tasktext'], 'clean', 'Task text is not clean')


  def check_logs_page(self, response):
      assert_true('syslog' in response.content, 'Syslogs not found in logs page')
      assert_true('<div class="tab-pane active" id="logsSysLog">' in response.content or
                  '<div class="tab-pane active" id="logsStdErr">' in response.content or  # Depending on Hadoop
                  '<div class="tab-pane active" id="logsStdOut">' in response.content,  # For jenkins
                  'Logs not found in single logs page')

  def test_job_single_logs_page(self):
    if webhdfs_conf.MR_CLUSTERS['default'].SUBMIT_TO.get():
        oozie_jobid = self.start_sleep_job()
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/single_logs' % (self.hadoop_job_id))
        start_time = time.time()
        while time.time() - start_time < 60:
            if 'syslog' in response.content:
                break
            else:
                response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/single_logs' % (self.hadoop_job_id))
        else:
            raise AssertionError('Timeout. Syslog is not in response')
        self.check_logs_page(response)
        OozieServerProvider.wait_until_completion(oozie_jobid)
    else:
        response = TestJobBrowserWithHadoop.client.get('/jobbrowser/jobs/%s/single_logs' % (self.hadoop_job_id))
        self.check_logs_page(response)



class TestMapReduce1NoHadoop:

  def test_acls_job(self):
    raise SkipTest
    job = MockMr1Job()

    assert_true(can_view_job('test', job))
    assert_true(can_modify_job('test', job))

    assert_false(can_view_job('test2', job))
    assert_false(can_modify_job('test2', job))


class MockMr1Job(Job):

  def __init__(self):
    self.is_mr2 = False
    self._full_job_conf = {
      'mapreduce.cluster.acls.enabled': True,
      'mapreduce.job.acl-modify-job': 'test',
      'mapreduce.job.acl-view-job': 'test'
    }


class TestMapReduce2NoHadoop:

  def setUp(self):
    raise SkipTest
    # Beware: Monkey patching
    if not hasattr(resource_manager_api, 'old_get_resource_manager_api'):
      resource_manager_api.old_get_resource_manager = resource_manager_api.get_resource_manager
    if not hasattr(resource_manager_api, 'old_get_mapreduce_api'):
      mapreduce_api.old_get_mapreduce_api = mapreduce_api.get_mapreduce_api
    if not hasattr(history_server_api, 'old_get_history_server_api'):
      history_server_api.old_get_history_server_api = history_server_api.get_history_server_api

    resource_manager_api.get_resource_manager = lambda: MockResourceManagerApi()
    mapreduce_api.get_mapreduce_api = lambda: MockMapreduceApi()
    history_server_api.get_history_server_api = lambda: HistoryServerApi()

    self.c = make_logged_in_client(is_superuser=False)
    grant_access("test", "test", "jobbrowser")
    self.user = User.objects.get(username='test')

    self.c2 = make_logged_in_client(is_superuser=False, username="test2")
    grant_access("test2", "test2", "jobbrowser")
    self.user2 = User.objects.get(username='test2')

    self.finish = [
        YARN_CLUSTERS['default'].SUBMIT_TO.set_for_testing(True),
        SHARE_JOBS.set_for_testing(False)
    ]
    assert_true(cluster.is_yarn())


  def tearDown(self):
    resource_manager_api.get_resource_manager = getattr(resource_manager_api, 'old_get_resource_manager')
    mapreduce_api.get_mapreduce_api = getattr(mapreduce_api, 'old_get_mapreduce_api')
    history_server_api.get_history_server_api = getattr(history_server_api, 'old_get_history_server_api')

    for f in self.finish:
      f()

  def test_jobs(self):
    response = self.c.get('/jobbrowser/?format=json')
    response_content = json.loads(response.content)
    assert_equal(len(response_content['jobs']), 4)

    response = self.c.get('/jobbrowser/jobs/?format=json&text=W=MapReduce-copy2')
    response_content = json.loads(response.content)
    assert_equal(len(response_content['jobs']), 1)

  def test_running_job(self):
    raise SkipTest
    response = self.c.get('/jobbrowser/jobs/application_1356251510842_0054')
    assert_true('job_1356251510842_0054' in response.content)
    assert_true('RUNNING' in response.content)

    response = self.c.get('/jobbrowser/jobs/job_1356251510842_0054')
    assert_true('job_1356251510842_0054' in response.content)
    assert_true('RUNNING' in response.content)

  def test_finished_job(self):
    raise SkipTest
    response = self.c.get('/jobbrowser/jobs/application_1356251510842_0009')
    assert_equal(response.context['job'].jobId, 'job_1356251510842_0009')

    response = self.c.get('/jobbrowser/jobs/job_1356251510842_0009')
    assert_equal(response.context['job'].jobId, 'job_1356251510842_0009')

  def test_spark_job(self):
    raise SkipTest
    response = self.c.get('/jobbrowser/jobs/application_1428442704693_0006')
    assert_equal(response.context['job'].jobId, 'application_1428442704693_0006')

  def test_yarn_job(self):
    raise SkipTest
    response = self.c.get('/jobbrowser/jobs/application_1428442704693_0007')
    assert_equal(response.context['job'].jobId, 'application_1428442704693_0007')

  def job_not_assigned(self):
    response = self.c.get('/jobbrowser/jobs/job_1356251510842_0009/job_not_assigned//my_url')
    assert_equal(response.context['jobid'], 'job_1356251510842_0009')
    assert_equal(response.context['path'], '/my_url')

    response = self.c.get('/jobbrowser/jobs/job_1356251510842_0009/job_not_assigned//my_url?format=json')
    result = json.loads(response.content)
    assert_equal(result['status'], 0)

  def test_acls_job(self):
    raise SkipTest
    response = self.c.get('/jobbrowser/jobs/job_1356251510842_0054') # Check in perm decorator
    assert_true(can_view_job('test', response.context['job']))
    assert_true(can_modify_job('test', response.context['job']))

    response2 = self.c2.get('/jobbrowser/jobs/job_1356251510842_0054')
    assert_true('don&#39;t have permission to access job' in response2.content, response2.content)

    assert_false(can_view_job('test2', response.context['job']))
    assert_false(can_modify_job('test2', response.context['job']))

  def test_kill_job(self):
    raise SkipTest
    job_id = 'application_1356251510842_0054'
    try:
      response = self.c.post('/jobbrowser/jobs/%s/kill?format=json' % job_id)
      assert_equal(json.loads(response.content), {"status": 0})
    finally:
      MockResourceManagerApi.APPS[job_id]['state'] = 'RUNNING'


class MockResourceManagerApi:
  APPS = {
    'application_1356251510842_0054': {
        u'finishedTime': 1356961070119,
        u'name': u'oozie:launcher:T=map-reduce:W=MapReduce-copy:A=Sleep:ID=0000004-121223003201296-oozie-oozi-W',
        u'amContainerLogs': u'http://localhost:8042/node/containerlogs/container_1356251510842_0054_01_000001/romain',
        u'clusterId': 1356251510842,
        u'trackingUrl': u'http://localhost:8088/proxy/application_1356251510842_0054/jobhistory/job/job_1356251510842_0054',
        u'amHostHttpAddress': u'localhost:8042',
        u'startedTime': 1356961057225,
        u'queue': u'default',
        u'state': u'RUNNING',
        u'elapsedTime': 12894,
        u'finalStatus': u'UNDEFINED',
        u'diagnostics': u'',
        u'progress': 100.0,
        u'trackingUI': u'History',
        u'id': u'application_1356251510842_0054',
        u'user': u'test',
        # For when the job is KILLED
        u'startTime': 1356961057226,
        u'finishTime': 1356961057226,
        u'applicationType': 'MAPREDUCE'
    },
    'application_1356251510842_0009': {
        u'finishedTime': 1356467118570,
        u'name': u'oozie:action:T=map-reduce:W=MapReduce-copy2:A=Sleep:ID=0000002-121223003201296-oozie-oozi-W',
        u'amContainerLogs': u'http://localhost:8042/node/containerlogs/container_1356251510842_0009_01_000001/romain',
        u'clusterId': 1356251510842,
        u'trackingUrl': u'http://localhost:8088/proxy/application_1356251510842_0009/jobhistory/job/job_1356251510842_0009',
        u'amHostHttpAddress': u'localhost:8042',
        u'startedTime': 1356467081121,
        u'queue': u'default',
        u'state': u'FINISHED',
        u'elapsedTime': 37449,
        u'finalStatus': u'SUCCEEDED',
        u'diagnostics': u'',
        u'progress': 100.0,
        u'trackingUI': u'History',
        u'id': u'application_1356251510842_0009',
        u'user': u'test',
        u'applicationType': 'MAPREDUCE'
    },
    'application_1428442704693_0006': {
        u'allocatedMB': 4096,
        u'allocatedVCores': 3,
        u'amContainerLogs': u'http://localhost:8042/node/containerlogs/container_1428442704693_0006_01_000001/erickt',
        u'amHostHttpAddress': u'localhost:8042',
        u'applicationTags': u'',
        u'applicationType': u'SPARK',
        u'clusterId': 1428442704693,
        u'diagnostics': u'',
        u'elapsedTime': 529040,
        u'finalStatus': u'UNDEFINED',
        u'finishedTime': 0,
        u'id': u'application_1428442704693_0006',
        u'memorySeconds': 2138468,
        u'name': u'Spark shell',
        u'numAMContainerPreempted': 0,
        u'numNonAMContainerPreempted': 0,
        u'preemptedResourceMB': 0,
        u'preemptedResourceVCores': 0,
        u'progress': 10.0,
        u'queue': u'root.erickt',
        u'runningContainers': 3,
        u'startedTime': 1428443335161,
        u'state': u'RUNNING',
        u'trackingUI': u'ApplicationMaster',
        u'trackingUrl': u'http://localhost:8088/proxy/application_1428442704693_0006/',
        u'user': u'test',
        u'vcoreSeconds': 1567,
    },
    'application_1428442704693_0007': {
        u'allocatedMB': -1,
        u'allocatedVCores': -1,
        u'applicationTags': u'',
        u'applicationType': u'YARN',
        u'clusterId': 1428442704693,
        u'diagnostics': u'',
        u'elapsedTime': 4056,
        u'finalStatus': u'SUCCEEDED',
        u'finishedTime': 1428454945371,
        u'id': u'application_1428442704693_0007',
        u'memorySeconds': 2290,
        u'name': u'UnmanagedAM',
        u'numAMContainerPreempted': 0,
        u'numNonAMContainerPreempted': 0,
        u'preemptedResourceMB': 0,
        u'preemptedResourceVCores': 0,
        u'progress': 100.0,
        u'queue': u'root.erickt',
        u'runningContainers': -1,
        u'startedTime': 1428454941315,
        u'state': u'FINISHED',
        u'trackingUI': u'History',
        u'trackingUrl': u'http://N/A',
        u'user': u'test',
        u'vcoreSeconds': 1,
    },
  }

  def __init__(self, oozie_url=None): pass

  def apps(self, **kwargs):
    return {
     'apps': {
       'app': [
         # RUNNING
         MockResourceManagerApi.APPS['application_1356251510842_0054'],
         # FINISHED
         MockResourceManagerApi.APPS['application_1356251510842_0009'],
         # SPARK
         MockResourceManagerApi.APPS['application_1428442704693_0006'],
         # YARN
         MockResourceManagerApi.APPS['application_1428442704693_0007'],
        ]
      }
    }

  def app(self, job_id):
    return {
      u'app': MockResourceManagerApi.APPS[job_id]
    }


class MockMapreduce2Api(object):
  """
  MockMapreduceApi and HistoryServerApi are very similar and inherit from it.
  """

  def __init__(self, oozie_url=None): pass

  def tasks(self, job_id):
    return {
      u'tasks': {
        u'task': [{
            u'finishTime': 1357153330271, u'successfulAttempt': u'attempt_1356251510842_0062_m_000000_0', u'elapsedTime': 1901, u'state': u'SUCCEEDED',
            u'startTime': 1357153328370, u'progress': 100.0, u'type': u'MAP', u'id': u'task_1356251510842_0062_m_000000'},
                  {
            u'finishTime': 0, u'successfulAttempt': u'', u'elapsedTime': 0, u'state': u'SCHEDULED', u'startTime': 1357153326322, u'progress': 0.0,
            u'type': u'REDUCE', u'id': u'task_1356251510842_0062_r_000000'}
        ]
      }
    }

  def conf(self, job_id):
    return {
      "conf" : {
        "path" : "hdfs://host.domain.com:9000/user/user1/.staging/job_1326232085508_0004/job.xml",
        "property" : [
           {
              "name" : "dfs.datanode.data.dir",
              "value" : "/home/hadoop/hdfs/data",
           }, {
              "name" : "mapreduce.job.acl-modify-job",
              "value" : "test",
           }, {
              "name" : "mapreduce.job.acl-view-job",
              "value" : "test",
           }
         ]
      }
    }

  def job_attempts(self, job_id):
    return {
       "jobAttempts" : {
          "jobAttempt" : [
             {
                "nodeId" : "host.domain.com:8041",
                "nodeHttpAddress" : "host.domain.com:8042",
                "startTime" : 1326238773493,
                "id" : 1,
                "logsLink" : "http://host.domain.com:8042/node/containerlogs/container_1326232085508_0004_01_000001",
                "containerId" : "container_1326232085508_0004_01_000001"
             }
          ]
       }
    }

  def task_attempts(self, job_id, task_id):
    return {
       "taskAttempts" : {
          "taskAttempt" : [
             {
                "elapsedMergeTime" : 47,
                "shuffleFinishTime" : 1326238780052,
                "assignedContainerId" : "container_1326232085508_0004_01_000003",
                "progress" : 100,
                "elapsedTime" : 0,
                "state" : "RUNNING",
                "elapsedShuffleTime" : 2592,
                "mergeFinishTime" : 1326238780099,
                "rack" : "/98.139.92.0",
                "elapsedReduceTime" : 0,
                "nodeHttpAddress" : "host.domain.com:8042",
                "type" : "REDUCE",
                "startTime" : 1326238777460,
                "id" : "attempt_1326232085508_4_4_r_0_0",
                "finishTime" : 0
             }
          ]
       }
    }

  def counters(self, job_id):
    return {
       "jobCounters" : {
          "id" : "job_1326232085508_4_4",
          "counterGroup" : [
             {
                "counterGroupName" : "org.apache.hadoop.mapreduce.lib.input.FileInputFormatCounter",
                "counter" : [
                   {
                      "reduceCounterValue" : 0,
                      "mapCounterValue" : 0,
                      "totalCounterValue" : 0,
                      "name" : "BYTES_READ"
                   }
                ]
             },
             {
                "counterGroupName" : "org.apache.hadoop.mapreduce.lib.output.FileOutputFormatCounter",
                "counter" : [
                   {
                      "reduceCounterValue" : 0,
                      "mapCounterValue" : 0,
                      "totalCounterValue" : 0,
                      "name" : "BYTES_WRITTEN"
                   }
                ]
             }
          ]
       }
    }

  def kill(self, job_id):
    job_id = job_id.replace('job', 'application')
    MockResourceManagerApi.APPS[job_id]['state'] = 'KILLED'
    return {}


class MockMapreduceApi(MockMapreduce2Api):
  def job(self, user, job_id):
    if '1356251510842_0009' not in job_id:
      job = {
          u'job': {
              u'reducesCompleted': 0, u'mapsRunning': 1, u'id': u'job_1356251510842_0054', u'successfulReduceAttempts': 0, u'successfulMapAttempts': 0,
              u'uberized': False, u'reducesTotal': 1, u'elapsedTime': 3426, u'mapsPending': 0, u'state': u'RUNNING', u'failedReduceAttempts': 0,
              u'mapsCompleted': 0, u'killedMapAttempts': 0, u'killedReduceAttempts': 0, u'runningReduceAttempts': 0, u'failedMapAttempts': 0, u'mapsTotal': 1,
              u'user': u'test', u'startTime': 1357152972886, u'reducesPending': 1, u'reduceProgress': 0.0, u'finishTime': 0,
              u'name': u'select avg(salary) from sample_07(Stage-1)', u'reducesRunning': 0, u'newMapAttempts': 0, u'diagnostics': u'', u'mapProgress': 0.0,
              u'runningMapAttempts': 1, u'newReduceAttempts': 1,
              # Does not seems to exist in API, we actually skip it in case.
              "acls" : [{
                  "value" : "test",
                  "name" : "mapreduce.job.acl-modify-job"
               }, {
                  "value" : "test",
                  "name" : "mapreduce.job.acl-view-job"
               }
              ],
          }
      }
      job['job']['id'] = job_id
      return job


class HistoryServerApi(MockMapreduce2Api):

  def __init__(self, oozie_url=None): pass

  def job(self, user, job_id):
    if '1356251510842_0054' == job_id:
      return {
          u'job': {
              u'reducesCompleted': 1, u'avgMapTime': 1798, u'avgMergeTime': 1479, u'id': job_id,
              u'successfulReduceAttempts': 1, u'successfulMapAttempts': 2, u'uberized': False, u'reducesTotal': 1,
              u'state': u'KILLED', u'failedReduceAttempts': 0, u'mapsCompleted': 2,
              u'killedMapAttempts': 0, u'diagnostics': u'', u'mapsTotal': 2, u'user': u'test',
              u'startTime': 1357151916268, u'avgReduceTime': 137,
              u'finishTime': 1357151923925, u'name': u'oozie:action:T=map-reduce:W=MapReduce-copy:A=Sleep:ID=0000004-121223003201296-oozie-oozi-W',
              u'avgShuffleTime': 1421, u'queue': u'default', u'killedReduceAttempts': 0, u'failedMapAttempts': 0
          }
      }
    else:      
      return {
          u'job': {
              u'reducesCompleted': 1, u'avgMapTime': 1798, u'avgMergeTime': 1479, u'id': u'job_1356251510842_0009',
              u'successfulReduceAttempts': 1, u'successfulMapAttempts': 2, u'uberized': False, u'reducesTotal': 1,
              u'state': u'SUCCEEDED', u'failedReduceAttempts': 0, u'mapsCompleted': 2,
              u'killedMapAttempts': 0, u'diagnostics': u'', u'mapsTotal': 2, u'user': u'test',
              u'startTime': 1357151916268, u'avgReduceTime': 137,
              u'finishTime': 1357151923925, u'name': u'oozie:action:T=map-reduce:W=MapReduce-copy:A=Sleep:ID=0000004-121223003201296-oozie-oozi-W',
              u'avgShuffleTime': 1421, u'queue': u'default', u'killedReduceAttempts': 0, u'failedMapAttempts': 0
          }
      }


def test_make_log_links():
  """
   Unit test for models.LinkJobLogs._make_links
  """

  # FileBrowser
  assert_equal(
      """<a href="/filebrowser/view/user/romain/tmp" target="_blank">hdfs://localhost:8020/user/romain/tmp</a>  &lt;dir&gt;""",
      LinkJobLogs._make_links('hdfs://localhost:8020/user/romain/tmp  <dir>')
  )
  assert_equal(
      """<a href="/filebrowser/view/user/romain/tmp" target="_blank">hdfs://localhost:8020/user/romain/tmp</a>&lt;dir&gt;""",
      LinkJobLogs._make_links('hdfs://localhost:8020/user/romain/tmp<dir>')
  )
  assert_equal(
      """output: <a href="/filebrowser/view/user/romain/tmp" target="_blank">/user/romain/tmp</a>  &lt;dir&gt;""",
      LinkJobLogs._make_links('output: /user/romain/tmp  <dir>')
  )
  assert_equal(
      'Successfully read 3760 records (112648 bytes) from: &quot;<a href="/filebrowser/view/user/hue/pig/examples/data/midsummer.txt" target="_blank">/user/hue/pig/examples/data/midsummer.txt</a>&quot;',
      LinkJobLogs._make_links('Successfully read 3760 records (112648 bytes) from: "/user/hue/pig/examples/data/midsummer.txt"')
  )
  assert_equal(
      'data,upper_case  MAP_ONLY  <a href="/filebrowser/view/user/romain/out/fffff" target="_blank">hdfs://localhost:8020/user/romain/out/fffff</a>,',
      LinkJobLogs._make_links('data,upper_case  MAP_ONLY  hdfs://localhost:8020/user/romain/out/fffff,')
  )
  assert_equal(
      'MAP_ONLY  <a href="/filebrowser/view/user/romain/out/fffff" target="_blank">hdfs://localhost:8020/user/romain/out/fffff</a>\n2013',
      LinkJobLogs._make_links('MAP_ONLY  hdfs://localhost:8020/user/romain/out/fffff\n2013')
  )
  assert_equal(
      ' <a href="/filebrowser/view/jobs.tsv" target="_blank">/jobs.tsv</a> ',
      LinkJobLogs._make_links(' /jobs.tsv ')
  )
  assert_equal(
      '<a href="/filebrowser/view/user/romain/job_pos_2012.tsv" target="_blank">hdfs://localhost:8020/user/romain/job_pos_2012.tsv</a>',
      LinkJobLogs._make_links('hdfs://localhost:8020/user/romain/job_pos_2012.tsv')
  )

  # JobBrowser
  assert_equal(
      """<a href="/jobbrowser/jobs/job_201306261521_0058" target="_blank">job_201306261521_0058</a>""",
      LinkJobLogs._make_links('job_201306261521_0058')
  )
  assert_equal(
      """Hadoop Job IDs executed by Pig: <a href="/jobbrowser/jobs/job_201306261521_0058" target="_blank">job_201306261521_0058</a>""",
      LinkJobLogs._make_links('Hadoop Job IDs executed by Pig: job_201306261521_0058')
  )
  assert_equal(
      """MapReduceLauncher  - HadoopJobId: <a href="/jobbrowser/jobs/job_201306261521_0058" target="_blank">job_201306261521_0058</a>""",
      LinkJobLogs._make_links('MapReduceLauncher  - HadoopJobId: job_201306261521_0058')
  )
  assert_equal(
      """- More information at: http://localhost:50030/jobdetails.jsp?jobid=<a href="/jobbrowser/jobs/job_201306261521_0058" target="_blank">job_201306261521_0058</a>""",
      LinkJobLogs._make_links('- More information at: http://localhost:50030/jobdetails.jsp?jobid=job_201306261521_0058')
  )
  assert_equal(
      """ Logging error messages to: job_201307091553_0028/attempt_201307091553_002""",
      LinkJobLogs._make_links(' Logging error messages to: job_201307091553_0028/attempt_201307091553_002')
  )
  assert_equal(
      """ pig-job_201307091553_0028.log""",
      LinkJobLogs._make_links(' pig-job_201307091553_0028.log')
  )
