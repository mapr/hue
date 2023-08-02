#!/bin/bash -ex

TIMESTAMP=${TIMESTAMP:-$(sh -c 'date "+%Y%m%d%H%M"')}
IMAGE_TAG="ubuntu18-java11-gcc7-hue:${TIMESTAMP}"

docker build --no-cache --rm -t="dfdkr.mip.storage.hpecorp.net/${IMAGE_TAG}" $@ ubuntu18
