#!/bin/bash -ex

GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`
IMAGE_TAG="centos8vault-java11-hue:${GIT_BRANCH}"

docker build --no-cache --rm -t="dfdkr.mip.storage.hpecorp.net/${IMAGE_TAG}" $@ centos8
