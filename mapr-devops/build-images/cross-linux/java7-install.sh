#!/bin/bash -ex

JAVA7_VERSION=80

wget http://artifactory.devops.lab/artifactory/buildmachine-files/cross-linux/jdk-7u${JAVA7_VERSION}-linux-x64.tar.gz
tar -zxvf jdk-7u${JAVA7_VERSION}-linux-x64.tar.gz -C /opt
ln -svf /opt/jdk1.7.0_${JAVA7_VERSION} /opt/jdk-1.7.0
rm -fv jdk-7*.tar.gz
