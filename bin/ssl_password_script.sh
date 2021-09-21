#!/bin/sh

MAPR_HOME=${MAPR_HOME:-"/opt/mapr"}

PASSWORD_PROP="ssl.server.keystore.password"
MAPRKEYCREDS_CONF="${MAPR_HOME}/conf/maprkeycreds.conf"
SSL_SERVER_CONF="${MAPR_HOME}/conf/ssl-server.xml"

if [ -e "$MAPRKEYCREDS_CONF" ]; then
    . "${MAPR_HOME}/server/common-ecosystem.sh" >/dev/null 2>&1
    keystorePass=$(getStorePw "$PASSWORD_PROP" "$MAPRKEYCREDS_CONF")
    rc=$?
    if [ $rc -ne 0 ]; then
        echo "Failed to extract keystore password from maprkeycreds.conf: '${keystorePass}'"
        exit 1
    fi
else
    keystorePass="$(fgrep -A 1 "$PASSWORD_PROP" "$SSL_SERVER_CONF" | tail -1 | sed 's/ *<value>//;s/<\/value>//')"
    if [ -z "$keystorePass" ]; then
        echo "Failed to extract keystore password from ssl-server.xml."
        exit 1
    fi
fi

echo "$keystorePass"
