#!/bin/bash

MAPR_CLUSTERS_CONF=/opt/mapr/conf/mapr-clusters.conf
MAPR_SSL_KEYSTORE_PATH=/opt/mapr/conf/ssl_keystore
DEST_KEYSTORE=keystore.p12
CERTIFCATE_PEM_FILE=cert.pem
SRC_STORE_PASSWD=mapr123
DEST_STORE_PASSWD=m@prt3ch777!!!S
OPENSSL_PKCS12_OUT=keystore.pem
OPENSSL_RSA_OUT=hue_private_keystore.pem
CERTIFICATEKEY=certificate


function find_cluster_name() {
if [[ -f $MAPR_CLUSTERS_CONF ]]; then
    LINE=$(head -n 1 $MAPR_CLUSTERS_CONF)
    # Trim leading and trailing whitespaces in cluster name
    echo "$LINE"| awk '{print $1}' | awk '{gsub(/^ +| +$/,"")} {print $0}'
else echo ''
fi
}


function find_certificate_key(){
    echo $(find_cluster_name)
}



if [[ -f $MAPR_SSL_KEYSTORE_PATH ]]; then
    CERTIFICATEKEY=$(find_certificate_key)
else
    exit 0
fi

if [[ -f $OPENSSL_RSA_OUT ]]; then
    exit 0;
fi

keytool -export -alias $CERTIFICATEKEY -keystore $MAPR_SSL_KEYSTORE_PATH -rfc -file $CERTIFCATE_PEM_FILE -storepass $SRC_STORE_PASSWD

if [[ $? -ne 0 ]]; then
    exit 0
fi

keytool -importkeystore -srckeystore $MAPR_SSL_KEYSTORE_PATH -destkeystore $DEST_KEYSTORE -srcstoretype JKS -deststoretype PKCS12 -srcstorepass $SRC_STORE_PASSWD -deststorepass ${DEST_STORE_PASSWD} -srcalias $CERTIFICATEKEY -destalias $CERTIFICATEKEY -srckeypass $SRC_STORE_PASSWD -destkeypass ${DEST_STORE_PASSWD} -noprompt

if [[ $? -ne 0 ]]; then
    exit 0
fi

openssl pkcs12 -in $DEST_KEYSTORE -out $OPENSSL_PKCS12_OUT -passin pass:${DEST_STORE_PASSWD} -passout pass:${DEST_STORE_PASSWD}

if [[ $? -ne 0 ]]; then
    exit 0
fi

openssl rsa -in $OPENSSL_PKCS12_OUT -out $OPENSSL_RSA_OUT -passin pass:${DEST_STORE_PASSWD}

if [[ $? -ne 0 ]]; then
    exit 0
fi

