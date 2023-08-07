#!/bin/sh

DRILL_VERSION_FILE="${MAPR_HOME}/drill/drillversion"

if [ -e "$DRILL_VERSION_FILE" ]; then
    DRILL_VERSION=$(cat "$DRILL_VERSION_FILE")
    DRILL_HOME=${DRILL_HOME:-"${MAPR_HOME}/drill/drill-${DRILL_VERSION}"}
    export DRILL_JAVA_OPTS=$(. "${DRILL_HOME}/bin/drill-config.sh" >/dev/null 2>&1; echo "$SQLLINE_JAVA_OPTS")
    export DRILL_CLASSPATH=$(. "${DRILL_HOME}/bin/drill-config.sh" >/dev/null 2>&1; echo "$CP")
fi

export DRILL_JAVA_OPTS="${DRILL_JAVA_OPTS} --add-opens=java.base/java.lang=ALL-UNNAMED"

# Drill JDBC Driver can also be set via environment variable:
# export DRILL_DRIVER="org.apache.drill.jdbc.Driver"
