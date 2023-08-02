#!/bin/bash -ex

TIMESTAMP=${TIMESTAMP:-$(sh -c 'date "+%Y%m%d%H%M"')}
IMAGE_TAG="centos8vault-java11-gcc8-hue:${TIMESTAMP}"

docker build --no-cache --rm -t="dfdkr.mip.storage.hpecorp.net/${IMAGE_TAG}" $@ centos8
