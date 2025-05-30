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

#
# Keycloak integation
#
queryjson() {
  "${HUE_HOME}/build/env/bin/python" -c "import json,sys; print(json.load(sys.stdin)${1})"
}

SSO_CONF=$(maprcli cluster getssoconf -json 2>/dev/null)
rc="$?"

if [ "$rc" != 0 ]; then
  export HUE_AUTH_BACKEND="desktop.auth.backend.PamBackend"
else
  export HUE_AUTH_BACKEND="desktop.auth.backend.PamBackend,desktop.auth.backend.OIDCBackend"

  clientid=$(echo "$SSO_CONF" | queryjson '["data"][0]["clientid"]')
  clientsecret=$(echo "$SSO_CONF" | queryjson '["data"][0]["clientsecret"]')
  issuerendpoint=$(echo "$SSO_CONF" | queryjson '["data"][0]["issuerendpoint"]')

  hue_url="http://$(hostname -f):8888"
  if [ "$hue_secure" = "true" ]; then
    hue_url="https://$(hostname -f):8888"
  fi

  export HUE_OIDC_RP_CLIENT_ID="$clientid"
  export HUE_OIDC_RP_CLIENT_SECRET="$clientsecret"
  export HUE_OIDC_OP_AUTHORIZATION_ENDPOINT="${issuerendpoint}/protocol/openid-connect/auth"
  export HUE_OIDC_OP_TOKEN_ENDPOINT="${issuerendpoint}/protocol/openid-connect/token"
  export HUE_OIDC_OP_USER_ENDPOINT="${issuerendpoint}/protocol/openid-connect/userinfo"
  export HUE_OIDC_OP_JWKS_ENDPOINT="${issuerendpoint}/protocol/openid-connect/certs"
  export HUE_LOGIN_REDIRECT_URL="${hue_url}/oidc/callback/"
  export HUE_LOGOUT_REDIRECT_URL="${issuerendpoint}/protocol/openid-connect/logout"
  export HUE_LOGIN_REDIRECT_URL_FAILURE="${hue_url}/hue/oidc_failed/"
fi
