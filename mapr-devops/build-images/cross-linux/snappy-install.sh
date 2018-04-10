#!/bin/bash -ex

# Why are we building snappy from source?
# To make sure that it is compiled with certain options.

SNAPPY_VERSION="1.0.5"

export CFLAGS="-fPIC ${CFLAGS}"
export CXXFLAGS="-fPIC ${CXXFLAGS}"

wget http://artifactory.devops.lab/artifactory/buildmachine-files/cross-linux/snappy-${SNAPPY_VERSION}.tar.gz
tar zxvf snappy-${SNAPPY_VERSION}.tar.gz -C /opt
ln -svf /opt/snappy-${SNAPPY_VERSION} /opt/snappy
pushd /opt/snappy
    CC="gcc -fPIC" ./configure --enable-shared --enable-static
    make
    make install
popd

rm -fv snappy*.tar.gz

