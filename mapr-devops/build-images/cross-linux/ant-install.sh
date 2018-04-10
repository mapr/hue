#!/bin/bash -ex

ANT_VERSION=1.8.2
# Ant version 1.8.2 - Later versions of ant will hit JDK-5101868 - http://bugs.java.com/bugdatabase/view_bug.do;?bug_id=5101868

wget http://artifactory.devops.lab/artifactory/buildmachine-files/cross-linux/apache-ant-${ANT_VERSION}-bin.tar.gz
tar -zxvf apache-ant-${ANT_VERSION}-bin.tar.gz -C /opt
ln -svf /opt/apache-ant-${ANT_VERSION} /opt/ant
rm -fv apache-ant-*.tar.gz
cp -fv /tmp/cross-linux/ant-profile.sh /etc/profile.d/.
