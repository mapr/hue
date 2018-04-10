#!/bin/bash -ex

export DEBIAN_FRONTEND=noninteractive
apt-get update


# prerequisites of cross-install
apt-get install -y \
    asciidoc \
    autoconf \
    docbook2x \
    fakeroot \
    gcc \
    g++ \
    gettext \
    install-info \
    libbz2-dev \
    libcurl4-gnutls-dev \
    lsb-core \
    make \
    openssh-client \
    openssh-server \
    rsync \
    sudo \
    tar \
    vim \
    wget \
    zlib1g-dev

/tmp/cross-linux/ant-install.sh
/tmp/cross-linux/git-install.sh
/tmp/cross-linux/java8-install.sh
cp -fv /tmp/cross-linux/java8-profile.sh /etc/profile.d/.
/tmp/cross-linux/maven-install.sh
/tmp/cross-linux/snappy-install.sh


# https://github.com/mapr/hue/tree/3.12.0-mapr-1707#development-prerequisites
# asciidoc / gcc / g++ / make installed in cross-linux prerequisites
# ant / mvn / jdk installed through cross-linux scripts
# python development packages are not required as we building python in Hue internally
apt-get install -y \
    libffi-dev \
    libkrb5-dev \
    libmysqlclient-dev \
    libsasl2-dev \
    libsasl2-modules-gssapi-mit \
    libsqlite3-dev \
    libssl-dev \
    libxml2-dev \
    libxslt-dev \
    libldap2-dev \
    libgmp3-dev \
    libz-dev
