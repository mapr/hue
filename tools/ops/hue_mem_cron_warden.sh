#!/bin/bash

KILL_ME=${KILL_ME:-5000}  # This is the number of MB at which it will kill.
                          # Starting with 5000(5gb)


export MAPR_HOME=${MAPR_HOME:-"/opt/mapr"}
export HUE_VERSION=${HUE_VERSION:-$(cat "${MAPR_HOME}/hue/hueversion")}
export HUE_HOME=${HUE_HOME:-"${MAPR_HOME}/hue/hue-${HUE_VERSION}"}


DAEMON_CONF="__PREFIX__/conf/daemon.conf"
MAPR_USER=${MAPR_USER:-$([ -f "$DAEMON_CONF" ] && grep "mapr.daemon.user" "$DAEMON_CONF" | cut -d '=' -f 2)}
MAPR_USER=${MAPR_USER:-"mapr"}

if [ -z "${MAPR_TICKETFILE_LOCATION}" ] && [ -e "${MAPR_HOME}/conf/mapruserticket" ]; then
    export MAPR_TICKETFILE_LOCATION="${MAPR_HOME}/conf/mapruserticket"
fi


STATUS_OUTPUT=$(sudo -u "$MAPR_USER" -E "${HUE_HOME}/bin/hue-server" status 2>&1)
STATUS=$?

if [ $STATUS != 0 ] ; then
    echo "Hue is not running"
    exit 0
fi

PID=$(echo "$STATUS_OUTPUT" | grep -o -E '[0-9]+' 2>/dev/null)

if ! kill -0 "$PID" ; then
    echo "Hue has wrong PID: $PID"
    exit 1
fi


MEM=$(ps --no-headers -o rss:1 "$PID")
MEM_MB=$(expr "$MEM" '/' 1024)


if [ "$MEM_MB" -gt "$KILL_ME" ] ; then
    sudo -u "$MAPR_USER" -E "${HUE_HOME}/bin/hue-server" stop
    sudo -u "$MAPR_USER" -E "${HUE_HOME}/bin/hue-server" start
fi
