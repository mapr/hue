#!/bin/bash -ex

# Why are we building Git from source?
# Because the git package that comes with some distributions is unable to clone from https sites. In particular, the git package for CentOS 6 cannot handle https.

GIT_VERSION=2.9.3

mkdir -pv /usr/share/info

wget http://artifactory.devops.lab/artifactory/buildmachine-files/cross-linux/git-${GIT_VERSION}.tar.gz
tar zxvf git-${GIT_VERSION}.tar.gz -C /opt
ln -sv /opt/git-${GIT_VERSION} /opt/git

pushd /opt/git
    make configure
    ./configure --prefix=/usr
    make all doc info
    make install install-doc install-html install-info
popd

rm -fv git*.tar.gz

git config --global user.name "DevOps @ MapR"
git config --global user.email "devops@mapr.com"

