#!/bin/sh

#
# Configure paths to make MapR-SASL bindings work
#
. "${MAPR_HOME}/conf/env.sh"

# Add path to libjvm.so into LD_LIBRARY_PATH
# Find JRE directory using MapR env.sh script
if [ -e "${JAVA_HOME}/jre/lib/amd64/server" ]; then
    # java_directory points to JDK
    libjvm_path="${JAVA_HOME}/jre/lib/amd64/server"
elif [ -e "${JAVA_HOME}/lib/amd64/server" ]; then
    # java_directory points to JRE (MapR env.sh can return JAVA_HOME pointing to JRE_HOME)
    libjvm_path="${JAVA_HOME}/lib/amd64/server"
elif [ -e "${JAVA_HOME}/lib/server" ]; then
    # Java 11
    libjvm_path="${JAVA_HOME}/lib/server"
fi
export LD_LIBRARY_PATH="${MAPR_HOME}/lib:${HUE_HOME}/build/env/lib:${libjvm_path}:${LD_LIBRARY_PATH}"

# Add /opt/mapr/libexp/maprsecurity.so into PYTHONPATH
export PYTHONPATH="${MAPR_HOME}/libexp:${PYTHONPATH}"


#
# Ensure MAPR_USER and MAPR_GROUP is set
#
DAEMON_CONF="${MAPR_HOME}/conf/daemon.conf"

MAPR_USER=${MAPR_USER:-$( [ -f "$DAEMON_CONF" ] && awk -F = '$1 == "mapr.daemon.user" { print $2 }' "$DAEMON_CONF" )}
MAPR_USER=${MAPR_USER:-"mapr"}
export MAPR_USER

MAPR_GROUP=${MAPR_GROUP:-$( [ -f "$DAEMON_CONF" ] && awk -F = '$1 == "mapr.daemon.group" { print $2 }' "$DAEMON_CONF" )}
MAPR_GROUP=${MAPR_GROUP:-"$MAPR_USER"}
export MAPR_GROUP


#
# Disable annoying warning that we are connecting to some service (like HttP-FS), without verifying its certificates.
#
export PYTHONWARNINGS="ignore:Unverified HTTPS request"


#
# Discover locations of other ecosystem components
#
if [ -e "${MAPR_HOME}/hive/hiveversion" ]; then
    HIVE_VERSION=${HIVE_VERSION:-$(cat "${MAPR_HOME}/hive/hiveversion")}
    HIVE_HOME=${HIVE_HOME:-"${MAPR_HOME}/hive/hive-${HIVE_VERSION}"}
    export HIVE_CONF_DIR=${HIVE_CONF_DIR:-"${HIVE_HOME}/conf"}
fi

if [ -e "${MAPR_HOME}/hbase/hbaseversion" ]; then
    HBASE_VERSION=${HBASE_VERSION:-$(cat "${MAPR_HOME}/hbase/hbaseversion")}
    HBASE_HOME=${HBASE_HOME:-"${MAPR_HOME}/hbase/hbase-${HBASE_VERSION}"}
    export HBASE_CONF_DIR=${HBASE_CONF_DIR:-"${HBASE_HOME}/conf"}
fi
