#!/bin/bash -ex

GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`

IMAGE_TAG="ubuntu12-java7-hue:${GIT_BRANCH}"

mkdir -pv ubuntu12/{setupfiles,cross-linux}
cp -rfv cross-linux/* ubuntu12/cross-linux/

docker build --rm -t="maprdocker.lab/${IMAGE_TAG}" $@ ubuntu12
