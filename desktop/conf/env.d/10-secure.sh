#!/bin/sh

hue_secure_file="${HUE_HOME}/desktop/conf/.isSecure"

if [ -e "$hue_secure_file" ] && [ $(cat "$hue_secure_file") = "true" ] ; then
  hue_secure="true"
fi

if [ "$hue_secure" = "true" ]; then
  export mechanism=${mechanism:-"MAPR-SECURITY"}
  export security_enabled=${security_enabled:-"true"}
  export ssl_cacerts=${ssl_cacerts:-"${MAPR_HOME}/conf/ssl_truststore.pem"}
  export ssl_validate=${ssl_validate:-"true"}
  export ssl_certificate=${ssl_certificate:-"${MAPR_HOME}/conf/ssl_keystore.pem"}
  export ssl_private_key=${ssl_private_key:-"${MAPR_HOME}/conf/ssl_keystore.pem"}
fi
