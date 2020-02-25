#!/bin/bash -ex

# Why are we building NodeJS from source?
# Because NodeJS package available through EPEL isn't working on CentOS 7.2 Vault because it's linked to some libraries from newer versions of CentOS 7.
# Also, we are using old NodeJS 8.x, because versions newer than 10.15.0 should be build with GCC-4.9, while CentOS 7.2 has GCC-4.8.5 and Ubuntu 14.04 has GCC-4.8.4
# because of the following commit: https://github.com/nodejs/node/commit/be14283bcda84d8bd29a61e460a6970227554b78

NODE_VERSION=8.17.0

wget http://artifactory.devops.lab/artifactory/buildmachine-files-copy/cross-linux/node-v${NODE_VERSION}.tar.gz
tar zxvf node-v${NODE_VERSION}.tar.gz -C /opt
ln -sv /opt/node-v${NODE_VERSION} /opt/node

pushd /opt/node
    ./configure --prefix=/usr
    make
    make install
popd

rm -fv node*.tar.gz
