#!/bin/bash -ex

GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`

IMAGE_TAG="centos7-java8-hue:${GIT_BRANCH}"

mkdir -pv centos7/{setupfiles,cross-linux}
pushd centos7/setupfiles
  rm -f *.rpm
  wget http://artifactory.devops.lab/artifactory/buildmachine-files/centos7/epel-release-latest-7.noarch.rpm
popd
cp -rfv cross-linux/* centos7/cross-linux/

docker build --rm -t="maprdocker.lab/${IMAGE_TAG}" $@ centos7
