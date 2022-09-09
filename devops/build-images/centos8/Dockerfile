FROM dfdkr.mip.storage.hpecorp.net/centos8vault-java11-gcc8

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

# libmysqlclient_r.a is needed to build MySQL-Python statically linked to native MySQL drivers.
# Replace "gcc -dumpversion" with "gcc -dumpfullversion" to fix build on GCC 7 and newer.
RUN yum remove -y mysql-devel && \
    yum install -y ncurses-devel && \
    curl -sL https://archive.mariadb.org/mariadb-5.5.68/source/mariadb-5.5.68.tar.gz | tar xz -C /opt && \
    ln -s /opt/mariadb-5.5.68 /opt/mariadb && \
    cd /opt/mariadb && \
    sed -i "s/-dumpversion/-dumpfullversion/g" ./BUILD/check-cpu && \
    bash ./BUILD/compile-pentium64 && \
    make install
ENV PATH /usr/local/mysql/bin:$PATH

# MHUE-327 Install dependency to build pyodbc
RUN yum install -y unixODBC-devel
