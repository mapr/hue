#!/bin/sh

MAPR_HOME=${MAPR_HOME:-"/opt/mapr"}
SSL_SERVER_CONFIG_FILE="${MAPR_HOME}/conf/ssl-server.xml"
SSL_KEYSTORE_PASSWORD_PROP="ssl.server.keystore.password"

get_property_value() {
  property_name="$1"
  sed -n '/'${property_name}'/{:a;N;/<\/value>/!ba {s|.*<value>\(.*\)</value>|\1|p}}' "$SSL_SERVER_CONFIG_FILE"
}

get_property_value "ssl.server.keystore.password"
