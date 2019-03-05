import os
import shlex
import threading

import logging
LOG = logging.getLogger(__name__)

from desktop.lib import paths
from librdbms.server.rdbms_base_lib import BaseRDBMSDataTable, BaseRDBMSResult, BaseRDMSClient
from librdbms.jdbc import Cursor

try:
  from py4j.java_gateway import JavaGateway
except ImportError, e:
  LOG.exception('Failed to import py4j')


DEFAULT_USERNAME = 'example'
DEFAULT_PASSWORD = 'example'
DEFAULT_IMPERSONATION = True
DEFAULT_PRINCIPAL = 'mapr/localhost@REALM'
DEFAULT_CLASSPATH = paths.get_desktop_root("libs/librdbms/drill-lib/*")
DEFAULT_JDBC_DRIVER = 'com.mapr.drill.jdbc41.Driver'
DEFAULT_ZK_DIRECTORY = '/drill/'


JAVA_GATEWAY_CACHE = None
JAVA_GATEWAY_CACHE_LOCK = threading.Lock()


def get_java_gateway(*args, **kwargs):
  global JAVA_GATEWAY_CACHE
  global JAVA_GATEWAY_CACHE_LOCK

  if JAVA_GATEWAY_CACHE is None:
    JAVA_GATEWAY_CACHE_LOCK.acquire()
    try:
      if 'die_on_exit' not in kwargs:
        kwargs['die_on_exit'] = True
      JAVA_GATEWAY_CACHE = JavaGateway.launch_gateway(*args, **kwargs)
    finally:
      JAVA_GATEWAY_CACHE_LOCK.release()

  return JAVA_GATEWAY_CACHE


class DataTable(BaseRDBMSDataTable): pass


class Result(BaseRDBMSResult): pass


class DrillClient(BaseRDMSClient):
  data_table_cls = DataTable
  result_cls = Result

  conn = None
  curs = None

  def __init__(self, query_server, user):
    super(DrillClient, self).__init__(query_server, user)

    connection_type = query_server['connection_type']
    drillbits = query_server['drillbits']
    zk_quorum = query_server['zk_quorum']
    zk_cluster_id = query_server['zk_cluster_id']
    zk_directory = query_server['options'].get('zk_directory', DEFAULT_ZK_DIRECTORY)

    mechanism = query_server['mechanism']
    username = query_server['username']
    password = query_server['password']
    impersonation = query_server['options'].get('impersonation', DEFAULT_IMPERSONATION)
    principal = query_server['options'].get('principal', DEFAULT_PRINCIPAL)

    classpath = ':'.join([
      query_server['options'].get('classpath', DEFAULT_CLASSPATH),
      os.environ.get('CLASSPATH', ''),
    ])

    jdbc_driver = query_server['options'].get('jdbc_driver', DEFAULT_JDBC_DRIVER)

    if connection_type == 'direct':
      connection_string = 'jdbc:drill:drillbit={}'.format(drillbits)
    elif connection_type == 'zookeeper':
      zk_path = os.path.normpath('/' + zk_directory + '/' + zk_cluster_id)
      zk_path = zk_path.replace('//', '/')  # normpath can return path with trailing slashes at beginning
      connection_string = 'jdbc:drill:zk={}{}'.format(zk_quorum, zk_path)
    else:
      raise Exception('{} is not allowed for connection_type'.format(connection_type))

    if mechanism == 'MAPR-SECURITY':
      connection_string = '{};auth=maprsasl'.format(connection_string)
    elif mechanism == 'GSSAPI':
      connection_string = '{};auth=kerberos;principal={}'.format(connection_string, principal)

    if impersonation:
      connection_string = '{};impersonation_target={}'.format(connection_string, user.username)

    self.connection_string = connection_string
    self.username = username
    self.password = password
    self.jdbc_driver = jdbc_driver

    javaopts = shlex.split(os.environ.get('MAPR_ECOSYSTEM_LOGIN_OPTS', ''))
    self.gateway = get_java_gateway(classpath=classpath, javaopts=javaopts)

    self.connect()

  def connect(self):
    if self.conn is None:
      self.gateway.jvm.Class.forName(self.jdbc_driver)
      self.conn = self.gateway.jvm.java.sql.DriverManager.getConnection(self.connection_string, self.username, self.password)

    if self.curs is None:
      self.curs = DrillCursor(self.conn)

  # Following method is never called
  def close(self):
    if self.curs is not None:
      self.curs.close()
      self.curs = None

    if self.conn is not None:
      self.conn.close()
      self.conn = None

  def use(self, database):
    self.curs.execute("USE `{}`".format(database))

  def execute_statement(self, statement):
    # Prevent from PARSE ERROR caused by semicolon at the end of statement (some JDBC-specific limitation)
    statement_cleaned = statement.strip().strip(';')
    columns = []
    self.curs.execute(statement_cleaned)
    if self.curs.description:
      # Column description contains following entries: COLUMN_NAME, DATA_TYPE, IS_NULLABLE
      columns = [{'name': column[0], 'type': column[1], 'comment': ''} for column in self.curs.description]
    return self.data_table_cls(self.curs, columns)

  def get_databases(self):
    self.curs.execute("SHOW SCHEMAS")
    databases = self.curs.fetchall()
    return databases

  def get_tables(self, database, table_names=[]):
    is_file = False
    self.curs.execute("SELECT `TYPE` FROM `INFORMATION_SCHEMA`.`SCHEMATA` WHERE `SCHEMA_NAME` = '{}'".format(database))
    schema_type_res = self.curs.fetchmany(1)
    if schema_type_res and schema_type_res[0] and schema_type_res[0][0] == 'file':
      is_file = True

    if is_file:
      # First field in row is file "name"
      self.curs.execute("SHOW FILES IN `{}`".format(database))
      tables = [row[0] for row in self.curs.fetchall()]
      # In Drill file system all directories and files that start with dot or underscore is ignored.
      tables = [table for table in tables if table[0] not in ('.', '_', )]
    else:
      # First field in row is "TABLE_SCHEMA", second is "TABLE_NAME"
      self.curs.execute("SHOW TABLES IN `{}`".format(database))
      tables = [row[1] for row in self.curs.fetchall()]

    if table_names:
      tables_filtered = []
      for table in tables:
        for table_name in table_names:
          if table_name in table:
            tables_filtered.append(table)
            break
      tables = tables_filtered
    return tables

  def get_columns(self, database, table, names_only=True):
    self.curs.execute("DESCRIBE `{}`.`{}`".format(database, table))
    columns = self.curs.fetchall()
    # Column description contains following entries: COLUMN_NAME, DATA_TYPE, IS_NULLABLE
    if names_only:
      columns = [column[0] for column in columns]
    else:
      columns = [{'name': column[0], 'type': column[1], 'comment': ''} for column in columns]
    return columns

  def get_sample_data(self, database, table, column=None, limit=100):
    column = "`{}`".format(column) if column else "*"
    query = "SELECT {} FROM `{}`.`{}` LIMIT {}".format(column, database, table, limit)
    return self.execute_statement(query)

  def explain(self, statement):
    if statement.upper().startswith('EXPLAIN PLAN FOR'):
      return self.execute_statement(statement)
    else:
      return self.execute_statement('EXPLAIN PLAN FOR ' + statement)


class DrillCursor(Cursor):
  # Since Hue execute only one query per DrillClient instance, and not trying to close DrillClient anywhere,
  # we need to close connection with following hack.
  # We can't do it in DrillClient.__del__, as destroying of DrillClient not guarantee that cursor is already not in use
  # (i.e. notebook.connectors.rdbms.RdbmsApi._execute and notebook.connectors.rdbms.RdbmsApi.execute).
  # Anyway, in current implementation only one cursor used with one connection, so it's save to close connection here.
  def __del__(self):
    if self.conn:
      self.conn.close()

    self.close()
