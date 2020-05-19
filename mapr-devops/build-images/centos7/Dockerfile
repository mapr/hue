FROM docker.artifactory.lab/centos7vault-java11-gcc7

# https://github.com/cloudera/hue/blob/release-4.6.0/docs/admin-manual/manual.md#centosrhel
# RUN yum install -y ant # from cross-linux/ant-install.sh
# RUN yum install -y asciidoc # from centos7.sh
# RUN yum install -y cyrus-sasl-devel # from cross-linux/cyrus-sasl-install.sh
# RUN yum install -y cyrus-sasl-gssapi # from cross-linux/cyrus-sasl-install.sh
# RUN yum install -y cyrus-sasl-plain # from cross-linux/cyrus-sasl-install.sh
# RUN yum install -y gcc # from Dockerfile
# RUN yum install -y gcc-c++ # from Dockerfile
RUN yum install -y krb5-devel
RUN yum install -y libffi-devel
RUN yum install -y libxml2-devel
RUN yum install -y libxslt-devel
# RUN yum install -y make # from Dockerfile
# RUN yum install -y mysql
# RUN yum install -y mysql-devel
# Install openldap-devel without installing cyrus-sasl and cyrus-sasl-devel:
RUN rpm -iv --nodeps $(yumdownloader -q --urls "openldap-devel*x86_64")
# RUN yum install -y python-devel # we building Python in Hue internally
RUN yum install -y sqlite-devel
RUN yum install -y gmp-devel

ENV C_INCLUDE_PATH /usr/local/include/sasl:$C_INCLUDE_PATH
ENV LIBRARY_PATH /usr/local/lib/sasl2/lib:$LIBRARY_PATH
ENV LD_LIBRARY_PATH /usr/local/lib/sasl2/lib:$LD_LIBRARY_PATH

# libmysqlclient_r.a is needed to build MySQL-Python statically linked to native MySQL drivers.
# Replace "gcc -dumpversion" with "gcc -dumpfullversion" to fix build on GCC 7.
RUN yum remove -y mariadb-devel mariadb-libs && \
    yum install -y ncurses-devel && \
    curl -L https://downloads.mariadb.org/f/mariadb-5.5.68/source/mariadb-5.5.68.tar.gz | tar xz -C /opt && \
    ln -s /opt/mariadb-5.5.68 /opt/mariadb && \
    cd /opt/mariadb && \
    sed -i "s/-dumpversion/-dumpfullversion/g" ./BUILD/check-cpu && \
    bash ./BUILD/compile-pentium64 && \
    make install
ENV PATH /usr/local/mysql/bin:$PATH

# MHUE-327 Install dependency to build pyodbc
RUN yum install -y unixODBC-devel
