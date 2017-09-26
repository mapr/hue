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
doRestart=${doRestart:-0}

# Initialize security-related variables
HUE_CERTIFICATES_DIR="${HUE_HOME}/keys"
HUE_CERTIFICATE_PEM_FILE="${HUE_CERTIFICATES_DIR}/cert.pem"
HUE_SRC_STORE_PASSWD='mapr123'
HUE_DEST_KEYSTORE="${HUE_CERTIFICATES_DIR}/keystore.p12"
HUE_DEST_STORE_PASSWD='m@prt3ch777!!!S'
HUE_OPENSSL_KEYSTORE="${HUE_CERTIFICATES_DIR}/keystore.pem"
HUE_OPENSSL_PRIVATE_KEYSTORE="${HUE_CERTIFICATES_DIR}/hue_private_keystore.pem"



# Parse options

USAGE="usage: $0 [-h] [-R] [--secure|--unsecure|--custom]"

while [ ${#} -gt 0 ] ; do
  case "$1" in
    --secure)
      isSecure=1;
      shift 1;;
    --unsecure)
      isSecure=0;
      shift 1;;
    --custom)
      isSecure=1;
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

function genCerts() {
  #if ! safeToRunMaprCLI ; then
  #  logErr 'Can not create security keys, because cluster is not configured.'
  #  return $RETURN_ERR_MAPRCLUSTER
  #fi


  CERTIFICATEKEY="$(getClusterName)"

  mkdir -p "${HUE_CERTIFICATES_DIR}"


  logInfo 'Generating certificate from keystore...'
  if [ -e "$HUE_CERTIFICATE_PEM_FILE" ] ; then
    logWarn "$HUE_CERTIFICATE_PEM_FILE already exists. Skipping it."
  else
    keytool -export -alias "${CERTIFICATEKEY}" -keystore "${MAPR_CLDB_SSL_KEYSTORE}" -rfc -file "${HUE_CERTIFICATE_PEM_FILE}" -storepass "${HUE_SRC_STORE_PASSWD}"

    if [ $? -ne 0 ] ; then
      logErr 'No certificate has been generated.'
      return $RETURN_ERR_OTHER
    fi
  fi


  logInfo 'Importing the keystore from JKS to PKCS12...'
  if [ -e "$HUE_DEST_KEYSTORE" ] ; then
    logWarn "$HUE_DEST_KEYSTORE already exists. Skipping it."
  else
    keytool -importkeystore -noprompt \
      -srckeystore "${MAPR_CLDB_SSL_KEYSTORE}" -destkeystore "${HUE_DEST_KEYSTORE}" \
      -srcstoretype JKS -deststoretype PKCS12 \
      -srcstorepass "${HUE_SRC_STORE_PASSWD}" -deststorepass "${HUE_DEST_STORE_PASSWD}" \
      -srcalias "${CERTIFICATEKEY}" -destalias "${CERTIFICATEKEY}" \
      -srckeypass "${HUE_SRC_STORE_PASSWD}" -destkeypass "${HUE_DEST_STORE_PASSWD}"

    if [ $? -ne 0 ] ; then
      logErr 'No keystore has been imported.'
      return $RETURN_ERR_OTHER
    fi
  fi


  logInfo 'Converting PKCS12 to pem using OpenSSL...'
  if [ -e "$HUE_OPENSSL_KEYSTORE" ] ; then
    logWarn "$HUE_OPENSSL_KEYSTORE already exists. Skipping it."
  else
    openssl pkcs12 -in "${HUE_DEST_KEYSTORE}" -out "${HUE_OPENSSL_KEYSTORE}" -passin "pass:${HUE_DEST_STORE_PASSWD}" -passout "pass:${HUE_DEST_STORE_PASSWD}"

    if [ $? -ne 0 ] ; then
      logErr 'No PKCS12 has been converted.'
      return $RETURN_ERR_OTHER
    fi
  fi

  logInfo 'Hiding the pass phrase so Python doesnt prompt for password while connecting to Hive...'
  if [ -e "$HUE_OPENSSL_PRIVATE_KEYSTORE" ] ; then
    logWarn "$HUE_OPENSSL_PRIVATE_KEYSTORE already exists. Skipping it."
  else
    openssl rsa -in ${HUE_OPENSSL_KEYSTORE} -out ${HUE_OPENSSL_PRIVATE_KEYSTORE} -passin "pass:${HUE_DEST_STORE_PASSWD}"

    if [ $? -ne 0 ] ; then
      logErr 'No RSA is used.'
      return $RETURN_ERR_OTHER
    fi
  fi


  logInfo 'Keys generated successfully.'

  return $RETURN_SUCCESS
}


function installWardenConfFile() {
  if  ! [ -d ${MAPR_CONF_DIR}/conf.d ]; then
    mkdir -p ${MAPR_CONF_DIR}/conf.d > /dev/null 2>&1
  fi

  cp ${HUE_HOME}/desktop/conf/warden.hue.conf ${MAPR_CONF_DIR}/conf.d/
  chown $MAPR_USER:$MAPR_GROUP ${MAPR_CONF_DIR}/conf.d/warden.hue.conf

  logInfo 'Warden conf for Hue copied.'
}



# Main part

# Configure security
if [ "$isSecure" == 1 ] ; then
  genCerts
  GEN_CERTS_RET=$?
  if [ $GEN_CERTS_RET -ne $RETURN_SUCCESS ] ; then
    logErr 'Can not configure Hue with -secure.'
    exit $GEN_CERTS_RET
  else
    doRestart=1
  fi
fi


# Back up hue.ini if it exists
HUE_INI_FILE="${HUE_HOME}/desktop/conf/hue.ini"
if [ -f "${HUE_INI_FILE}" ] ; then
  cp "${HUE_INI_FILE}" "${HUE_INI_FILE}.bak-$(date '+%Y%m%d-%H%M%S')"
fi


# Change permissions
chown -R $MAPR_USER:$MAPR_GROUP "$HUE_HOME"


# Ask Warden to restart Hue if needed
if [ "$doRestart" == 1 ] && [ "$isOnlyRoles" != 1 ] ; then
  mkdir -p ${MAPR_CONF_DIR}/restart
  echo "maprcli node services -action restart -name hue -nodes $(hostname)" > "${MAPR_CONF_DIR}/restart/hue-${HUE_VERSION}.restart"
  chown $MAPR_USER:$MAPR_GROUP "${MAPR_CONF_DIR}/restart/hue-${HUE_VERSION}.restart"
fi


installWardenConfFile


# remove state file
if [ -f "$HUE_HOME/desktop/conf/.not_configured_yet" ]; then
  rm -f "$HUE_HOME/desktop/conf/.not_configured_yet"
fi



exit $RETURN_SUCCESS
