#!/bin/bash -ex


yum install -y /tmp/setupfiles/epel-release-latest-7.noarch.rpm
sed -i "s/mirrorlist=https/mirrorlist=http/" /etc/yum.repos.d/epel.repo
yum clean all


# prerequisites of cross-install
yum install -y \
    asciidoc \
    autoconf \
    bzip2-devel \
    curl-devel \
    docbook2X \
    gcc \
    gcc-c++ \
    gettext-devel \
    openssh-clients \
    openssh-server \
    make \
    perl-ExtUtils-MakeMaker \
    redhat-lsb-core \
    rpm-build \
    rsync \
    sudo \
    tar \
    tk-devel \
    vim-enhanced \
    wget \
    xmlto \
    zlib-devel

pushd /usr/bin
    ln -sf db2x_docbook2texi docbook2x-texi
popd

/tmp/cross-linux/ant-install.sh
/tmp/cross-linux/git-install.sh
/tmp/cross-linux/java8-install.sh
cp -fv /tmp/cross-linux/java8-profile.sh /etc/profile.d/.
/tmp/cross-linux/maven-install.sh
/tmp/cross-linux/snappy-install.sh


# MHUE-63
cp /tmp/setupfiles/MariaDB.repo /etc/yum.repos.d/
yum clean all
yum install -y MariaDB-devel

# https://github.com/mapr/hue/tree/3.12.0-mapr-1707#development-prerequisites
# asciidoc / gcc / gcc-c++ / make installed in cross-linux prerequisites
# ant / mvn / jdk installed through cross-linux scripts
# python development packages are not required as we building python in Hue internally
yum install -y \
    cyrus-sasl-devel \
    cyrus-sasl-gssapi \
    cyrus-sasl-plain \
    krb5-devel \
    libffi-devel \
    libxml2-devel \
    libxslt-devel \
    openldap-devel \
    openssl-devel \
    sqlite-devel \
    gmp-devel
