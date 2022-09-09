#!/bin/bash -ex

GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`
IMAGE_TAG="centos7-java8-hue:${GIT_BRANCH}"

rm -f centos7/setupfiles/*.rpm
wget -P centos7/setupfiles/ http://dfaf.mip.storage.hpecorp.net/artifactory/buildmachine-files/centos7/epel-release-latest-7.noarch.rpm

docker build --no-cache --rm -t="dfdkr.mip.storage.hpecorp.net/${IMAGE_TAG}" $@ centos7
