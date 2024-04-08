%undefine __check_files
%define _binaries_in_noarch_packages_terminate_build 0
%global __prelink_undo_cmd %{nil}


summary:     MapR
license:     Hewlett Packard Enterprise, CopyRight
Vendor:      Hewlett Packard Enterprise, <ezmeral_software_support@hpe.com>
name:        mapr-hue
version:     __RELEASE_VERSION__
release:     1
prefix:      /
group:       MapR
buildarch:   noarch
obsoletes:   mapr-hue < 3.10.0 , mapr-hue-base < 3.10.0

requires: mapr-client, mapr-hadoop-util, redhat-lsb-core, cyrus-sasl-gssapi, cyrus-sasl-plain, libxml2, libxslt, sqlite, zlib, libcrypto.so.1.1, libssl.so.1.1

AutoReqProv: no


%description
Ezmeral Ecosystem Pack - Hue package
Tag: __RELEASE_BRANCH__
Commit: __GIT_COMMIT__


%clean
echo "NOOP"


%files
__PREFIX__/


%pretrans
# Ensure there is no running Hue before upgrade
if [ -e "__PREFIX__/hue" ]; then
    if [ -z "${MAPR_TICKETFILE_LOCATION}" ] && [ -e "__PREFIX__/conf/mapruserticket" ]; then
        export MAPR_TICKETFILE_LOCATION="__PREFIX__/conf/mapruserticket"
    fi

    DAEMON_CONF="__PREFIX__/conf/daemon.conf"
    MAPR_USER=${MAPR_USER:-$([ -f "$DAEMON_CONF" ] && grep "mapr.daemon.user" "$DAEMON_CONF" | cut -d '=' -f 2)}
    MAPR_USER=${MAPR_USER:-"mapr"}

    HUE_VERSION=$(cat "__PREFIX__/hue/hueversion")
    HUE_HOME="__PREFIX__/hue/hue-${HUE_VERSION}"

    if sudo -u $MAPR_USER -E "${HUE_HOME}/bin/hue-server" status >/dev/null 2>&1; then
        RESULT=$(sudo -u $MAPR_USER -E "${HUE_HOME}/bin/hue-server" stop 2>&1)
        STATUS=$?
        if [ $STATUS -ne 0 ] ; then
            echo "$RESULT"
        fi
    fi

    #
    # MHUE-550: Check for Hue processes that weren't stopped succesfully / affects mapr-hue-4.11.0.0
    #
    # Wait for graceful stop first
    ghost_pid=$(ps ax -o pid:1,command | grep '/[o]pt/mapr/hue.*run.*server' | cut -d ' ' -f 1 | head -n1)
    if [ -n "$ghost_pid" ]; then
        kill -2 "$ghost_pid"
        sleep 2
    fi
    ghost_pids=$(ps ax -o pid:1,command | grep '/[o]pt/mapr/hue.*run.*server' | cut -d ' ' -f 1)
    for pid in $ghost_pids; do
        echo "Unable to stop hue runcpserver, pid ${pid}, forcefully stopping!"
        kill -9 "$pid"
    done
fi



%pre
if [ "$1" = "2" ]; then
    OLD_TIMESTAMP=$(rpm --queryformat='%%{VERSION}' -q mapr-hue)
    OLD_VERSION=$(echo "$OLD_TIMESTAMP" | grep -o '^[0-9]*\.[0-9]*\.[0-9]*')

    OLD_TIMESTAMP_FILE="%{_localstatedir}/lib/rpm-state/mapr-hue-old-timestamp"
    OLD_VERSION_FILE="%{_localstatedir}/lib/rpm-state/mapr-hue-old-version"

    STATE_DIR="$(dirname $OLD_TIMESTAMP_FILE)"
    if [ ! -d "$STATE_DIR" ]; then
        mkdir -p "$STATE_DIR"
    fi

    echo "$OLD_TIMESTAMP" > "$OLD_TIMESTAMP_FILE"
    echo "$OLD_VERSION" > "$OLD_VERSION_FILE"

    #
    # Backup of old configuration files
    #
    OLD_DIR="__PREFIX__/hue/hue-${OLD_VERSION}"
    BCK_DIR="__PREFIX__/hue/hue-${OLD_TIMESTAMP}"

    # Workaround for MHUE-428
    if [ -e "$BCK_DIR" ]; then
        BCK_DIR="${BCK_DIR}-2"
    fi

    CONF_SRC_DST="${OLD_DIR}/desktop/conf/ ${BCK_DIR}/desktop/
${OLD_DIR}/desktop/desktop.db ${BCK_DIR}/desktop/
${OLD_DIR}/bin/env.d/ ${BCK_DIR}/bin/"
    echo "$CONF_SRC_DST" | while read CONF_SRC CONF_DST; do
        mkdir -p "$CONF_DST"
        if [ -e "$CONF_SRC" ]; then
            cp -r "$CONF_SRC" "$CONF_DST"
        fi
    done
fi


%post
if [ "$1" = "1" ]; then
  touch "__INSTALL_3DIGIT__/desktop/conf/.not_configured_yet"
fi

# Init symlinks for missing libraries
for LIB in "__INSTALL_3DIGIT__"/ext/libcompat/*; do
    LIBNAME=$(basename $LIB)
    if ! ldconfig -p | grep -q "$LIBNAME "; then
        . "$LIB"
        for LIB_PATH in $LIB_LOOKUP_PATH; do
            if [ -e "$LIB_PATH" ] && [ ! -e "__INSTALL_3DIGIT__/build/env/lib/${LIBNAME}" ]; then
                ln -sf "$LIB_PATH" "__INSTALL_3DIGIT__/build/env/lib/${LIBNAME}"
                break
            fi
        done
        unset LIB_LOOKUP_PATH
    fi
done

# Init configuration files from templates
for CONF in "__INSTALL_3DIGIT__"/desktop/conf.dist/* ; do
    CONFNAME=$(basename $CONF)
    if [ ! -e "__INSTALL_3DIGIT__/desktop/conf/$CONFNAME" ] ; then
        cp -Rp $CONF "__INSTALL_3DIGIT__/desktop/conf/$CONFNAME"
    fi
done


%preun
# Stop service before uninstall.
# (Code in %pretrans is not executed on uninstall.)
if [ -z "${MAPR_TICKETFILE_LOCATION}" ] && [ -e "__PREFIX__/conf/mapruserticket" ]; then
    export MAPR_TICKETFILE_LOCATION="__PREFIX__/conf/mapruserticket"
fi

DAEMON_CONF="__PREFIX__/conf/daemon.conf"
MAPR_USER=${MAPR_USER:-$([ -f "$DAEMON_CONF" ] && grep "mapr.daemon.user" "$DAEMON_CONF" | cut -d '=' -f 2)}
MAPR_USER=${MAPR_USER:-"mapr"}

HUE_VERSION=$(cat "__PREFIX__/hue/hueversion")
HUE_HOME="__PREFIX__/hue/hue-${HUE_VERSION}"

if sudo -u $MAPR_USER -E "${HUE_HOME}/bin/hue-server" status &>/dev/null ; then
    RESULT=$(sudo -u $MAPR_USER -E "${HUE_HOME}/bin/hue-server" stop 2>&1)
    STATUS=$?
    if [ $STATUS -ne 0 ] ; then
        echo "$RESULT"
    fi
fi

# Remove temporary Python files.
if [ -d __INSTALL_3DIGIT__ ]; then
  find __INSTALL_3DIGIT__ -name '*.py[co]' -delete
fi


%postun
if [ "$1" == "0" ]; then
    rm -Rf "__PREFIX__/hue/"
fi

if [ -f "__PREFIX__/conf/conf.d/warden.hue.conf" ]; then
    rm -Rf "__PREFIX__/conf/conf.d/warden.hue.conf"
fi


%posttrans
OLD_TIMESTAMP_FILE="%{_localstatedir}/lib/rpm-state/mapr-hue-old-timestamp"
OLD_VERSION_FILE="%{_localstatedir}/lib/rpm-state/mapr-hue-old-version"

# This files will exist only on upgrade
if [ -e "$OLD_TIMESTAMP_FILE" ] && [ -e "$OLD_VERSION_FILE" ]; then
    OLD_TIMESTAMP=$(cat "$OLD_TIMESTAMP_FILE")
    OLD_VERSION=$(cat "$OLD_VERSION_FILE")

    rm "$OLD_TIMESTAMP_FILE" "$OLD_VERSION_FILE"

    # Remove directory with old version
    NEW_VERSION=$(cat __PREFIX__/hue/hueversion)

    if [ "$OLD_VERSION" != "$NEW_VERSION" ]; then
        rm -rf "__PREFIX__/hue/hue-${OLD_VERSION}"
    fi
fi
