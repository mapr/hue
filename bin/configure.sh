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
RETURN_ERR_MAPR_HOME=1
RETURN_ERR_ARGS=2
RETURN_ERR_MAPRCLUSTER=3



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

MAPR_USER=${MAPR_USER:-$( [ -f "$DAEMON_CONF" ] && awk -F = '$1 == "mapr.daemon.user" { print $2 }' "$DAEMON_CONF" )}
MAPR_USER=${MAPR_USER:-"mapr"}
export MAPR_USER
MAPR_GROUP=${MAPR_GROUP:-$( [ -f "$DAEMON_CONF" ] && awk -F = '$1 == "mapr.daemon.group" { print $2 }' "$DAEMON_CONF" )}
MAPR_GROUP=${MAPR_GROUP:-"$MAPR_USER"}
export MAPR_GROUP

# Initialize HUE_HOME
HUE_VERSION=$(cat "${MAPR_HOME}/hue/hueversion")
HUE_HOME=${HUE_HOME:-"${MAPR_HOME}/hue/hue-${HUE_VERSION}"}
MAPR_CONF_DIR=${MAPR_CONF_DIR:-"$MAPR_HOME/conf"}

# Initialize arguments
isOnlyRoles=${isOnlyRoles:-0}

# Initialize security-related variables
HUE_SECURE_FILE="${HUE_HOME}/desktop/conf/.isSecure"

# Warden related variables
WARDEN_HUE_SRC="${HUE_HOME}/desktop/conf.dist/warden.hue.conf"
WARDEN_HUE_CONF="${MAPR_CONF_DIR}/conf.d/warden.hue.conf"

WARDEN_HEAPSIZE_MIN_KEY="service.heapsize.min"
WARDEN_HEAPSIZE_MAX_KEY="service.heapsize.max"
WARDEN_HEAPSIZE_PERCENT_KEY="service.heapsize.percent"
WARDEN_RUNSTATE_KEY="service.runstate"

# Other
HUE_LOG_DIR="${HUE_HOME}/logs"
HUE_LOG_INITIAL_DB_MIGRATION="${HUE_LOG_DIR}/initial-db-migration.log"



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
      exit 0
      ;;
    *)
      # Invalid arguments passed
      echo "${USAGE}"
      exit $RETURN_ERR_ARGS
  esac
done



# Functions

perm_scripts() {
  chmod 0700 "${HUE_HOME}/bin/env.d/20secure"
}

perm_confs() {
  if [ -e "${HUE_HOME}/desktop/conf/hue.ini" ]; then
    logInfo "Setting permissions of Hue confs to 0600, since there are can be database/LDAP/etc. passwords."
    chmod 0600 "${HUE_HOME}/desktop/conf/hue.ini"
  fi
}


conf_get_property() {
  local conf_file="$1"
  local property_name="$2"
  local delim="="
  grep "^\s*${property_name}" "${conf_file}" | sed "s|^\s*${property_name}\s*${delim}\s*||"
}

conf_set_property() {
  local conf_file="$1"
  local property_name="$2"
  local property_value="$3"
  local delim="="
  if grep -q "^\s*${property_name}\s*${delim}" "${conf_file}"; then
    # modify property
    sed -i -r "s|^\s*${property_name}\s*${delim}.*$|${property_name}${delim}${property_value}|" "${conf_file}"
  else
    echo "${property_name}${delim}${property_value}" >> "${conf_file}"
  fi
}

setup_warden_conf() {
  local curr_heapsize_min
  local curr_heapsize_max
  local curr_heapsize_percent
  local curr_runstate

  if [ -f "$WARDEN_HUE_CONF" ]; then
    curr_heapsize_min=$(conf_get_property "$WARDEN_HUE_CONF" "$WARDEN_HEAPSIZE_MIN_KEY")
    curr_heapsize_max=$(conf_get_property "$WARDEN_HUE_CONF" "$WARDEN_HEAPSIZE_MAX_KEY")
    curr_heapsize_percent=$(conf_get_property "$WARDEN_HUE_CONF" "$WARDEN_HEAPSIZE_PERCENT_KEY")
    curr_runstate=$(conf_get_property "$WARDEN_HUE_CONF" "$WARDEN_RUNSTATE_KEY")
  fi

  cp "$WARDEN_HUE_SRC" "$WARDEN_HUE_CONF"

  [ -n "$curr_heapsize_min" ] && conf_set_property "$WARDEN_HUE_CONF" "$WARDEN_HEAPSIZE_MIN_KEY" "$curr_heapsize_min"
  [ -n "$curr_heapsize_max" ] && conf_set_property "$WARDEN_HUE_CONF" "$WARDEN_HEAPSIZE_MAX_KEY" "$curr_heapsize_max"
  [ -n "$curr_heapsize_percent" ] && conf_set_property "$WARDEN_HUE_CONF" "$WARDEN_HEAPSIZE_PERCENT_KEY" "$curr_heapsize_percent"
  [ -n "$curr_runstate" ] && conf_set_property "$WARDEN_HUE_CONF" "$WARDEN_RUNSTATE_KEY" "$curr_runstate"

  chown $MAPR_USER:$MAPR_GROUP "$WARDEN_HUE_CONF"

  logInfo 'Warden conf for Hue copied.'
}


create_restart_file(){
  mkdir -p ${MAPR_CONF_DIR}/restart
  cat > "${MAPR_CONF_DIR}/restart/hue-${HUE_VERSION}.restart" <<'EOF'
#!/bin/bash
MAPR_HOME="${MAPR_HOME:-/opt/mapr}"
MAPR_USER="${MAPR_USER:-mapr}"
if [ -z "${MAPR_TICKETFILE_LOCATION}" ] && [ -e "${MAPR_HOME}/conf/mapruserticket" ]; then
    export MAPR_TICKETFILE_LOCATION="${MAPR_HOME}/conf/mapruserticket"
fi
sudo -u $MAPR_USER -E maprcli node services -action restart -name hue -nodes $(hostname)
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


init_db_and_user() {
  oldpwd="$PWD"
  cd "$HUE_HOME"
  sudo -u "$MAPR_USER" "${HUE_HOME}/bin/hue" migrate --run-syncdb --fake-initial &&
  sudo -u "$MAPR_USER" "${HUE_HOME}/bin/hue" shell <<EOF
from django.contrib.auth import get_user_model
from useradmin.models import get_default_user_group
User = get_user_model()

try:
  user = User.objects.get(username='$MAPR_USER')
except:
  user = User.objects.create(id=1, username='$MAPR_USER')
  user.set_password('mapr')
  user.is_superuser = True
  default_group = get_default_user_group()
  if default_group is not None:
    user.groups.add(default_group)
  user.save()
EOF
  ret=$?
  cd "$oldpwd"
  return $ret
}



# Main part

if [ "$isOnlyRoles" == 1 ] ; then
  # Configure security
  oldSecure=$(read_secure)
  updSecure="false"
  if [ -n "$isSecure" ] && [ "$isSecure" != "$oldSecure" ] ; then
    updSecure="true"
  fi

  perm_scripts
  perm_confs

  if [ "$updSecure" = "true" ] ; then
    write_secure "$isSecure"
  fi

  chown_component
  sudo -u "$MAPR_USER" mkdir -p "$HUE_LOG_DIR"

  logInfo "Syncing database."
  initdb_out=$(init_db_and_user 2>&1)
  initdb_res=$?
  if [ $initdb_res -ne 0 ] ; then
    logErr "Failed to perform database sync or failed to set '$MAPR_USER' to be Hue admin."
  fi
  echo "$initdb_out" | sudo -u "$MAPR_USER" tee -a "$HUE_LOG_INITIAL_DB_MIGRATION" >/dev/null

  setup_warden_conf

  # remove state file
  if [ -f "$HUE_HOME/desktop/conf/.not_configured_yet" ] ; then
    rm -f "$HUE_HOME/desktop/conf/.not_configured_yet"
  elif [ "$updSecure" = "true" ] ; then
    create_restart_file
  fi
fi



exit 0
