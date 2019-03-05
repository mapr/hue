#!/bin/sh

export MAPR_HOME=${MAPR_HOME:-"/opt/mapr"}
export HUE_VERSION=${HUE_VERSION:-$(cat "${MAPR_HOME}/hue/hueversion")}
export HUE_HOME=${HUE_HOME:-"${MAPR_HOME}/hue/hue-${HUE_VERSION}"}

# Activate virtualenv
. "${HUE_HOME}/build/env/bin/activate"

for envfile in ${HUE_HOME}/bin/env.d/* ; do
    . "$envfile"
done
