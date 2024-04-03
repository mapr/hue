#!/bin/bash
set -e

SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")
. "${SCRIPT_DIR}/_initialize_package_variables.sh"
. "${SCRIPT_DIR}/_utils.sh"

build_hue() {
  rm -rf "$PKG_INSTALL_ROOT"

  PYTHON_PREFIX="${PKG_INSTALL_ROOT}/build/python"
  mkdir -p "${BUILD_ROOT}/python"
  curl -s "$PYTHON_SRC" | tar xJ --strip-components=1 -C "${BUILD_ROOT}/python"
  pushd "${BUILD_ROOT}/python"
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

  mkdir -p "$BUILD_ROOT"
  mv "$PKG_INSTALL_ROOT" "${BUILD_ROOT}/build"
}

main() {
  echo "Cleaning '${BUILD_ROOT}' dir..."
  rm -rf "$BUILD_ROOT"

  echo "Building project..."
  build_hue

  echo "Preparing directory structure..."
  setup_role "mapr-hue"

  setup_package "mapr-hue"

  echo "Building packages..."

  if [ "$OS" = "redhat" ] && echo "$PLATFORM_ID" | grep -q "el8"; then
    export TIMESTAMP=$(expr "$TIMESTAMP" - 1)
  fi
  build_package "mapr-hue"

  echo "Resulting packages:"
  find "$DIST_DIR" -exec readlink -f {} \;
}

main
