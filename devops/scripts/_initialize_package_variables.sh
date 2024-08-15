#!/bin/bash

GIT_COMMIT=$(git log -1 --pretty=format:"%H")
INSTALLATION_PREFIX=${INSTALLATION_PREFIX:-"/opt/mapr"}
PKG_NAME=${PKG_NAME:-"hue"}
PKG_VERSION=${PKG_VERSION:-"4.11.0.100"}
PKG_3DIGIT_VERSION=$(echo "$PKG_VERSION" | cut -d '.' -f 1-3)
TIMESTAMP=${TIMESTAMP:-$(sh -c 'date "+%Y%m%d%H%M"')}
PKG_INSTALL_ROOT=${PKG_INSTALL_ROOT:-"${INSTALLATION_PREFIX}/${PKG_NAME}/${PKG_NAME}-${PKG_3DIGIT_VERSION}"}

DIST_DIR=${DIST_DIR:-"devops/dist"}
# rpmbuild does not work properly when relatve path specified here
BUILD_ROOT=${BUILD_ROOT:-"$(pwd)/devops/buildroot"}

PYTHON_VERSION_LONG=${PYTHON_VER_LONG:-"3.8.19"}
PYTHON_VERSION=$(echo "$PYTHON_VERSION_LONG" | cut -d '.' -f 1-2)
PYTHON_VER="python${PYTHON_VERSION}"
PYTHON_SRC=${PYTHON_SRC:-"https://www.python.org/ftp/python/${PYTHON_VERSION_LONG}/Python-${PYTHON_VERSION_LONG}.tar.xz"}
