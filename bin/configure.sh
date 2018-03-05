#!/bin/bash
#######################################################################
# Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved
#######################################################################
#
# Configure script for Hue
#
# This script is normally run by the core configure.sh to setup Hue
# during install. If it is run standalone, need to correctly initialize
# the variables that it normally inherits from the master configure.sh
#######################################################################

RETURN_SUCCESS=0
RETURN_ERR_MAPR_HOME=1
RETURN_ERR_ARGS=2
RETURN_ERR_MAPRCLUSTER=3
RETURN_ERR_OTHER=4



# Initialize API and globals
MAPR_HOME=${MAPR_HOME:-/opt/mapr}

. ${MAPR_HOME}/server/common-ecosystem.sh  2> /dev/null

if [ $? -ne 0 ] ; then
  echo '[ERROR] MAPR_HOME seems to not be set correctly or mapr-core not installed.'
  exit $RETURN_ERR_MAPR_HOME
fi

{ set +x; } 2>/dev/null

initCfgEnv

# Get MAPR_USER and MAPR_GROUP
DAEMON_CONF="${MAPR_HOME}/conf/daemon.conf"
if [ -z "$MAPR_USER" ] ; then
  if [ -f "$DAEMON_CONF" ]; then
    MAPR_USER=$( awk -F = '$1 == "mapr.daemon.user" { print $2 }' "$DAEMON_CONF" )
  else
    #Hue installation on edge node (not on cluster)
    MAPR_USER=`logname`
  fi
fi
if [ -z "$MAPR_GROUP" ] ; then
  if [ -f "$DAEMON_CONF" ]; then
    MAPR_GROUP=$( awk -F = '$1 == "mapr.daemon.group" { print $2 }' "$DAEMON_CONF" )
  else
    MAPR_GROUP="$MAPR_USER"
  fi
fi

# Initialize HUE_HOME
HUE_VERSION=$(cat "${MAPR_HOME}/hue/hueversion")
HUE_HOME=${HUE_HOME:-"${MAPR_HOME}/hue/hue-${HUE_VERSION}"}
MAPR_CONF_DIR=${MAPR_CONF_DIR:-"$MAPR_HOME/conf"}

# Initialize arguments
isOnlyRoles=${isOnlyRoles:-0}
isSecure=${isSecure:-"false"}

# Initialize security-related variables
HUE_SECURE_FILE="${HUE_HOME}/desktop/conf/.isSecure"

HUE_KEYS_DIR="${HUE_HOME}/keys"
HUE_PEM_CERT_FILE="${HUE_KEYS_DIR}/cert.pem"
HUE_SRC_STORE_PASSWD='mapr123'
HUE_TMP_STORE="${HUE_KEYS_DIR}/keystore.p12"
HUE_TMP_STORE_PASSWD='m@prt3ch777!!!S'
HUE_PEM_KEY_FILE="${HUE_KEYS_DIR}/hue_private_keystore.pem"



# Parse options

USAGE="usage: $0 [-h] [-R] [--secure|--unsecure|--customSecure] [-EC <options>]"

while [ ${#} -gt 0 ] ; do
  case "$1" in
    --secure)
      isSecure="true";
      shift 1;;
    --unsecure)
      isSecure="false";
      shift 1;;
    --customSecure)
      isSecure="custom";
      shift 1;;
    -R)
      isOnlyRoles=1;
      shift 1;;
    -EC)
      for i in $2 ; do
        case $i in
          -R) isOnlyRoles=1 ;;
          *) : ;; # unused in Hue
        esac
      done
      shift 2;;
    -h)
      echo "${USAGE}"
      exit $RETURN_SUCCESS
      ;;
    *)
      # Invalid arguments passed
      echo "${USAGE}"
      exit $RETURN_ERR_ARGS
  esac
done



# Functions

gen_keys() {
  CERTIFICATEKEY="$(getClusterName)"

  mkdir -p "${HUE_KEYS_DIR}"


  logInfo 'Generating certificate from keystore...'
  if [ -e "$HUE_PEM_CERT_FILE" ] ; then
    logWarn "$HUE_PEM_CERT_FILE already exists. Skipping it."
  else
    keytool -export -alias "${CERTIFICATEKEY}" -keystore "${MAPR_CLDB_SSL_KEYSTORE}" -rfc -file "${HUE_PEM_CERT_FILE}" -storepass "${HUE_SRC_STORE_PASSWD}"

    if [ $? -ne 0 ] ; then
      logErr 'No certificate has been generated.'
      return $RETURN_ERR_OTHER
    fi
  fi


  logInfo 'Importing the keystore from JKS to PKCS12...'
  if [ -e "$HUE_TMP_STORE" ] ; then
    logWarn "$HUE_TMP_STORE already exists. Skipping it."
  else
    keytool -importkeystore -noprompt \
      -srckeystore "${MAPR_CLDB_SSL_KEYSTORE}" -destkeystore "${HUE_TMP_STORE}" \
      -srcstoretype JKS -deststoretype PKCS12 \
      -srcstorepass "${HUE_SRC_STORE_PASSWD}" -deststorepass "${HUE_TMP_STORE_PASSWD}" \
      -srcalias "${CERTIFICATEKEY}" -destalias "${CERTIFICATEKEY}" \
      -srckeypass "${HUE_SRC_STORE_PASSWD}" -destkeypass "${HUE_TMP_STORE_PASSWD}"

    if [ $? -ne 0 ] ; then
      logErr 'No keystore has been imported.'
      return $RETURN_ERR_OTHER
    fi
  fi


  # Remove obsolete keystore.pem file with passphrase
  if [ -e "${HUE_KEYS_DIR}/keystore.pem" ]; then
    rm -f "${HUE_KEYS_DIR}/keystore.pem"
  fi


  logInfo 'Converting PKCS12 to pem using OpenSSL...'
  if [ -e "$HUE_PEM_KEY_FILE" ] ; then
    logWarn "$HUE_PEM_KEY_FILE already exists. Skipping it."
  else
    openssl pkcs12 -in ${HUE_TMP_STORE} -passin "pass:${HUE_TMP_STORE_PASSWD}" -passout "pass:${HUE_TMP_STORE_PASSWD}" |
      openssl rsa -passin "pass:${HUE_TMP_STORE_PASSWD}" -out ${HUE_PEM_KEY_FILE}

    if [ $? -ne 0 ] ; then
      logErr 'No PKCS12 has been converted.'
      return $RETURN_ERR_OTHER
    fi
  fi


  logInfo "Remove temporary PKCS12 keystore file..."
  if [ -e "${HUE_TMP_STORE}" ]; then
    rm -f "${HUE_TMP_STORE}"
  fi


  logInfo 'Keys generated successfully.'

  return $RETURN_SUCCESS
}


perm_keys() {
  if [ -e "${HUE_PEM_CERT_FILE}" ]; then
    chmod 0600 "${HUE_PEM_CERT_FILE}"
  fi
  if [ -e "${HUE_PEM_KEY_FILE}" ]; then
    chmod 0400 "${HUE_PEM_KEY_FILE}"
  fi
}

perm_scripts() {
  chmod 0700 "${HUE_HOME}/bin/configure.sh"

  # TODO: remove this after removing of bin/secure.sh
  chmod 0700 "${HUE_HOME}/bin/secure.sh"
}

perm_confs() {
  if [ -e "${HUE_HOME}/desktop/conf/hue.ini" ]; then
    logInfo "Setting permissions of Hue confs to 0600, since there are can be database/LDAP/etc. passwords."
    chmod 0600 "${HUE_HOME}/desktop/conf/hue.ini"
  fi
}


install_warden_file() {
  if  ! [ -d ${MAPR_CONF_DIR}/conf.d ]; then
    mkdir -p ${MAPR_CONF_DIR}/conf.d > /dev/null 2>&1
  fi

  cp ${HUE_HOME}/desktop/conf/warden.hue.conf ${MAPR_CONF_DIR}/conf.d/
  chown $MAPR_USER:$MAPR_GROUP ${MAPR_CONF_DIR}/conf.d/warden.hue.conf

  logInfo 'Warden conf for Hue copied.'
}


create_restart_file(){
  mkdir -p ${MAPR_CONF_DIR}/restart
  cat > "${MAPR_CONF_DIR}/restart/hue-${HUE_VERSION}.restart" <<'EOF'
#!/bin/bash
MAPR_HOME="${MAPR_HOME:-/opt/mapr}"
if [ -z "${MAPR_TICKETFILE_LOCATION}" ] && [ -e "${MAPR_HOME}/conf/mapruserticket" ]; then
    export MAPR_TICKETFILE_LOCATION="${MAPR_HOME}/conf/mapruserticket"
fi
maprcli node services -action restart -name hue -nodes $(hostname)
EOF
  chmod +x "${MAPR_CONF_DIR}/restart/hue-${HUE_VERSION}.restart"
  chown -R $MAPR_USER:$MAPR_GROUP "${MAPR_CONF_DIR}/restart/hue-${HUE_VERSION}.restart"
}


chown_component() {
  chown -R $MAPR_USER:$MAPR_GROUP "$HUE_HOME"
}


read_secure() {
  [ -e "${HUE_SECURE_FILE}" ] && cat "${HUE_SECURE_FILE}"
}

write_secure() {
  echo "$1" > "${HUE_SECURE_FILE}"
}



# Main part

if [ "$isOnlyRoles" == 1 ]; then
  # Configure security
  if [ "$isSecure" = "true" ] ; then
    gen_keys
    GEN_CERTS_RET=$?
    if [ $GEN_CERTS_RET -ne $RETURN_SUCCESS ] ; then
      logErr 'Can not configure Hue with --secure.'
      exit $GEN_CERTS_RET
    fi
  fi

  perm_keys
  perm_scripts
  perm_confs

  chown_component

  install_warden_file

  if [ "$(read_secure)" != "$isSecure" ]; then
    write_secure "$isSecure"
    create_restart_file
  fi

  # remove state file
  if [ -f "$HUE_HOME/desktop/conf/.not_configured_yet" ]; then
    rm -f "$HUE_HOME/desktop/conf/.not_configured_yet"
  fi
fi



exit $RETURN_SUCCESS
