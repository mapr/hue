#!/bin/bash

MAPR_CLUSTERS_CONF=/opt/mapr/conf/mapr-clusters.conf
MAPR_SSL_KEYSTORE_PATH=/opt/mapr/conf/ssl_keystore
SECURE='false'
DEST_KEYSTORE=keystore.p12
CERTIFCATE_PEM_FILE=cert.pem
SRC_STORE_PASSWD=mapr123
DEST_STORE_PASSWD=m@prt3ch777!!!S
OPENSSL_PKCS12_OUT=keystore.pem
OPENSSL_RSA_OUT=hue_private_keystore.pem
CERTIFICATEKEY=certificate

function log {
    echo "[$(date +"%Y-%m-%d %H:%M:%S.%3N")]: $*"
}

function find_cluster_name() {
if [[ -f $MAPR_CLUSTERS_CONF ]]; then
    LINE=$(head -n 1 $MAPR_CLUSTERS_CONF)
    # Trim leading and trailing whitespaces in cluster name
    echo "$LINE"| awk '{print $1}' | awk '{gsub(/^ +| +$/,"")} {print $0}'
else echo ''
fi
}

function find_is_secure_enabled() {
if [[ -f $MAPR_CLUSTERS_CONF ]];then
    LINE=$(head -n 1 $MAPR_CLUSTERS_CONF)
    # Trim leading and trailing whitespaces and changing output to lower case
    echo "$LINE"| awk '{print $2}'| awk -F'=' '{print $2}' | awk '{print tolower($0)}' | awk '{gsub(/^ +| +$/,"")} {print $0}'
else echo 'false'
fi
}

function find_certificate_key(){
    echo $(find_cluster_name)
}

SECURE=$(find_is_secure_enabled)

log '[INFO] secure = '$SECURE

if [[ $SECURE == 'true' ]]; then
    log '[INFO] Using existing ssl_keystore: '$MAPR_SSL_KEYSTORE_PATH
    if [[ -f $MAPR_SSL_KEYSTORE_PATH ]]; then
        CERTIFICATEKEY=$(find_certificate_key)
    else
        log '[ERROR] No MapR SSL keystore found here: '$MAPR_SSL_KEYSTORE_PATH
        exit 0
    fi
    log '[INFO] CERTIFICATEKEY = '$CERTIFICATEKEY
else
    log '[INFO] Done.'
    exit 0
fi

if [[ -f $OPENSSL_RSA_OUT ]]; then
    log '[INFO] File '$(readlink -e $OPENSSL_RSA_OUT)' already exists. Exiting.'
    exit 0;
fi

log '[INFO] Generating certificate from keystore...'
keytool -export -alias $CERTIFICATEKEY -keystore $MAPR_SSL_KEYSTORE_PATH -rfc -file $CERTIFCATE_PEM_FILE -storepass $SRC_STORE_PASSWD

if [[ $? -ne 0 ]]; then
    log '[ERROR] No certificate has been generated.'
    exit 0
fi

log '[INFO] Importing the keystore from JKS to PKCS12...'
keytool -importkeystore -srckeystore $MAPR_SSL_KEYSTORE_PATH -destkeystore $DEST_KEYSTORE -srcstoretype JKS -deststoretype PKCS12 -srcstorepass $SRC_STORE_PASSWD -deststorepass ${DEST_STORE_PASSWD} -srcalias $CERTIFICATEKEY -destalias $CERTIFICATEKEY -srckeypass $SRC_STORE_PASSWD -destkeypass ${DEST_STORE_PASSWD} -noprompt

if [[ $? -ne 0 ]]; then
    log '[ERROR] No keystore has been imported.'
    exit 0
fi

log '[INFO] Converting pkcs12 to pem using OpenSSL...'
openssl pkcs12 -in $DEST_KEYSTORE -out $OPENSSL_PKCS12_OUT -passin pass:${DEST_STORE_PASSWD} -passout pass:${DEST_STORE_PASSWD}

if [[ $? -ne 0 ]]; then
    log '[ERROR] No pkcs12 has been converted.'
    exit 0
fi

log '[INFO] Hiding the pass phrase so Python doesnt prompt for password while connecting to Hive...'
openssl rsa -in $OPENSSL_PKCS12_OUT -out $OPENSSL_RSA_OUT -passin pass:${DEST_STORE_PASSWD}

if [[ $? -ne 0 ]]; then
    log '[ERROR] No rsa is used.'
    exit 0
fi

log '[INFO] See output here: '$(readlink -e $OPENSSL_RSA_OUT)
log '[INFO] Done.'
