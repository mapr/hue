#!/bin/bash -ex

GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`
IMAGE_TAG="ubuntu16-java11-gcc7-hue:${GIT_BRANCH}"

docker build --no-cache --rm -t="docker.artifactory.lab/${IMAGE_TAG}" $@ ubuntu16
