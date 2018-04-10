#!/bin/bash -ex

JAVA8_VERSION=144

wget http://artifactory.devops.lab/artifactory/buildmachine-files/cross-linux/jdk-8u${JAVA8_VERSION}-linux-x64.tar.gz
tar -zxvf jdk-8u${JAVA8_VERSION}-linux-x64.tar.gz -C /opt
ln -svf /opt/jdk1.8.0_${JAVA8_VERSION} /opt/jdk-1.8.0
rm -fv jdk-8*.tar.gz
