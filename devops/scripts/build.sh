#!/bin/bash
set -e

SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")
. "${SCRIPT_DIR}/_initialize_package_variables.sh"

do_build() {
  echo "Cleaning '${DIST_DIR}' dir..."
  rm -rf "$DIST_DIR"

  echo "Building project..."
  build_hue
  if [ "$?" -ne 0 ]; then
    return 1
  fi

  rc=0
  rpmbuild >/dev/null 2>&1 || rc=$?
  if [ "$rc" -ne 127 ]; then
    echo "Building rpm..."
    build_hue_rpm
  fi

  rc=0
  dpkg-deb >/dev/null 2>&1 || rc=$?
  if [ "$rc" -ne 127 ]; then
    echo "Building deb..."
    build_hue_deb
  fi

  find "$DIST_DIR" -mindepth 1 -not \( -name '*.deb' -o -name '*.rpm' \) -delete
  echo "Resulting packages:"
  find "$DIST_DIR" -type f -name '*.rpm' -exec readlink -f {} \;
  find "$DIST_DIR" -type f -name '*.deb' -exec readlink -f {} \;
}

build_hue() {
  rm -rf "$PKG_INSTALL_ROOT"

  PYTHON_PREFIX="${PKG_INSTALL_ROOT}/build/python"

  mkdir -p python
  curl -s "$PYTHON_SRC" | tar xJ --strip-components=1 -C python
  pushd python
    ./configure --prefix="$PYTHON_PREFIX"
    make install
  popd

  export INSTALL_DIR="$PKG_INSTALL_ROOT"
  export PATH="${PYTHON_PREFIX}/bin:$PATH"
  export ROOT=$(pwd)
  export PYTHON_VER
  export SYS_PYTHON="${PYTHON_PREFIX}/bin/${PYTHON_VER}"
  export PYTHON_H="${PYTHON_PREFIX}/include/${PYTHON_VER}/Python.h"
  make install

  pushd "$PKG_INSTALL_ROOT"
    python3 -m pip install virtualenv-make-relocatable==0.0.1
    bash tools/relocatable.sh
  popd
  
  find "$PKG_INSTALL_ROOT" -name '*.py[co]' -delete
  rm -rf \
    "${PKG_INSTALL_ROOT}/logs" \
    "${PKG_INSTALL_ROOT}/apps/logs"

  mkdir -p "${DIST_DIR}"
  mv "$PKG_INSTALL_ROOT" "${DIST_DIR}/build"
}

build_hue_rpm() {
  mkdir -p ${DIST_DIR_RPM}/{SPECS,INSTALL,SOURCES,BUILD,RPMS/noarch} "$DIST_INSTALL_ROOT_RPM"
  mv -T "${DIST_DIR}/build" "$DIST_INSTALL_ROOT_RPM"

  create_role "${DIST_ROOT_RPM}${INSTALLATION_PREFIX}"
  create_version "${DIST_ROOT_RPM}${INSTALLATION_PREFIX}"

  cp -r devops/specs/rpm/*.spec "${DIST_DIR_RPM}/SPECS"
  pack_rpm "$DIST_DIR_RPM"
}

build_hue_deb() {
  mkdir -p "${DIST_DIR_DEB}/DEBIAN" "${DIST_INSTALL_ROOT_DEB}"
  mv -T "${DIST_DIR}/build" "$DIST_INSTALL_ROOT_DEB"

  create_role "${DIST_ROOT_DEB}${INSTALLATION_PREFIX}"
  create_version "${DIST_ROOT_DEB}${INSTALLATION_PREFIX}"

  cp -r devops/specs/deb/* "${DIST_DIR_DEB}/DEBIAN"
  pack_deb $DIST_DIR_DEB
}

create_role() {
  PREFIX="$1"
  mkdir -p "${PREFIX}/roles"
  touch "${PREFIX}/roles/${PKG_NAME}"
}

create_version() {
  PREFIX="$1"
  mkdir -p "${PREFIX}/${PKG_NAME}"
  echo "$PKG_3DIGIT_VERSION" > "${PREFIX}/${PKG_NAME}/${PKG_NAME}version"

  ln -sr "${PREFIX}/${PKG_NAME}/${PKG_NAME}-${PKG_3DIGIT_VERSION}" "${PREFIX}/${PKG_NAME}/current"
}

pack_rpm() {
  RMP_DIR="$1"
  replace_build_variables "${RMP_DIR}/SPECS"
  rpmbuild --bb --define "_topdir ${RMP_DIR}" --buildroot="${RMP_DIR}/SOURCES" ${RMP_DIR}/SPECS/*
  mv ${RMP_DIR}/RPMS/*/*rpm "$DIST_DIR"
}

pack_deb() {
  DEB_DIR="$1"
  replace_build_variables "${DEB_DIR}/DEBIAN"
  find "$DEB_DIR" -type f -exec md5sum \{\} \; 2>/dev/null |
    sed -e "s|${DEB_DIR}||" -e "s| \/| |" |
    grep -v DEBIAN > "${DEB_DIR}/DEBIAN/md5sums"
  echo "" >> "${DEB_DIR}/DEBIAN/control"
  dpkg-deb --build "$DEB_DIR" "$DIST_DIR"
}

replace_build_variables() {
  REPLACE_DIR="$1"
  find "$REPLACE_DIR" -type f \
    -exec sed -i "s|__GIT_COMMIT__|${GIT_COMMIT}|g" {} \; \
    -exec sed -i "s|__PREFIX__|${INSTALLATION_PREFIX}|g" {} \; \
    -exec sed -i "s|__VERSION__|${PKG_VERSION}|g" {} \; \
    -exec sed -i "s|__VERSION_3DIGIT__|${PKG_3DIGIT_VERSION}|g" {} \; \
    -exec sed -i "s|__RELEASE_BRANCH__|${PACKAGE_INFO_BRANCH}|g" {} \; \
    -exec sed -i "s|__RELEASE_VERSION__|${PKG_VERSION}.${TIMESTAMP}|g" {} \; \
    -exec sed -i "s|__INSTALL_3DIGIT__|${PKG_INSTALL_ROOT}|g" {} \;
}

do_build
