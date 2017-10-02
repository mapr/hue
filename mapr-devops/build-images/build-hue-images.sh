#!/usr/bin/env bash

cd centos7
docker build -t maprdocker.lab/centos7-java8-hue:hue-3.12.0-mapr .
docker push maprdocker.lab/centos7-java8-hue:hue-3.12.0-mapr

cd centos6
docker build -t maprdocker.lab/centos6-java7-hue:hue-3.12.0-mapr .
docker push maprdocker.lab/centos6-java7-hue:hue-3.12.0-mapr

cd ubuntu14
docker build -t maprdocker.lab/ubuntu14-java8-hue:hue-3.12.0-mapr .
docker push maprdocker.lab/ubuntu14-java8-hue:hue-3.12.0-mapr

echo "Hue images Built"
