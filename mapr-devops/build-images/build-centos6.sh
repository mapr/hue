#!/bin/bash -ex

GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`

IMAGE_TAG="centos6-java7-hue:${GIT_BRANCH}"

mkdir -pv centos6/{setupfiles,cross-linux}
cp -rfv cross-linux/* centos6/cross-linux/

docker build --rm -t="maprdocker.lab/${IMAGE_TAG}" $@ centos6
