#!/bin/bash -ex

GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`

IMAGE_TAG="ubuntu14-java8-hue:${GIT_BRANCH}"

mkdir -pv ubuntu14/{setupfiles,cross-linux}
cp -rfv cross-linux/* ubuntu14/cross-linux/

docker build --rm -t="maprdocker.lab/${IMAGE_TAG}" $@ ubuntu14
