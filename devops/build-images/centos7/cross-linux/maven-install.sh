#!/bin/bash -ex

MAVEN_VERSION=3.3.9

wget http://dfaf.mip.storage.hpecorp.net/artifactory/buildmachine-files/cross-linux/apache-maven-${MAVEN_VERSION}-bin.tar.gz
tar -zxvf apache-maven-${MAVEN_VERSION}-bin.tar.gz -C /opt
ln -svf /opt/apache-maven-${MAVEN_VERSION} /opt/maven
rm -fv apache-maven-*.tar.gz
