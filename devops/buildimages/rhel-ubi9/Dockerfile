ARG BASE_IMAGE
FROM ${BASE_IMAGE}

# https://github.com/cloudera/hue/blob/release-4.6.0/docs/admin-manual/manual.md#centosrhel
# RUN yum install -y ant # from cross-linux/ant-install.sh
# RUN yum install -y asciidoc # from centos7vault/centos7.sh
RUN yum install -y cyrus-sasl-devel
RUN yum install -y cyrus-sasl-gssapi
RUN yum install -y cyrus-sasl-plain
# RUN yum install -y gcc # from centos7vault/Dockerfile
# RUN yum install -y gcc-c++ # from centos7vault/Dockerfile
RUN yum install -y krb5-devel
RUN yum install -y libffi-devel
RUN yum install -y libxml2-devel
RUN yum install -y libxslt-devel
# RUN yum install -y make # from centos7vault/Dockerfile
# RUN yum install -y mysql
# RUN yum install -y mysql-devel
RUN yum install -y openldap-devel
# RUN yum install -y python-devel # we building Python in Hue internally
RUN yum install -y sqlite-devel
RUN yum install -y gmp-devel

RUN yum remove -y nodejs && \
    curl -sL https://rpm.nodesource.com/setup_14.x | bash - && \
    yum install -y nodejs

# MHUE-327 Install dependency to build pyodbc
RUN yum install -y \
    https://dl.rockylinux.org/vault/rocky/9.2/AppStream/x86_64/os/Packages/u/unixODBC-2.3.9-4.el9.x86_64.rpm \
    https://dl.rockylinux.org/vault/rocky/9.2/CRB/x86_64/os/Packages/u/unixODBC-devel-2.3.9-4.el9.x86_64.rpm
