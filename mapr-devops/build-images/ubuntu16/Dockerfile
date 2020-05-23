FROM docker.artifactory.lab/ubuntu16-java11-gcc7

ENV DEBIAN_FRONTEND noninteractive
RUN apt update

# https://github.com/cloudera/hue/blob/release-4.6.0/docs/admin-manual/manual.md#ubuntu
# RUN apt install -y git # from cross-linux/git-install.sh
# RUN apt install -y ant # from cross-linux/ant-install.sh
# RUN apt install -y gcc # from ubuntu16/Dockerfile
# RUN apt install -y g++ # from ubuntu16/Dockerfile
RUN apt install -y libffi-dev
RUN apt install -y libkrb5-dev
# RUN apt install -y libmysqlclient-dev
RUN apt install -y libsasl2-dev
RUN apt install -y libsasl2-modules-gssapi-mit
RUN apt install -y libsqlite3-dev
# RUN apt install -y libssl-dev # from ubuntu16/ubuntu16-install.sh
RUN apt install -y libxml2-dev
RUN apt install -y libxslt-dev
# RUN apt install -y make # from ubuntu16/ubuntu16-install.sh
# RUN apt install -y maven # from cross-linux/maven-install.sh
RUN apt install -y libldap2-dev
# RUN apt install -y python-dev # we building Python in Hue internally
# RUN apt install -y python-setuptools # we building Python in Hue internally
RUN apt install -y libgmp3-dev
# RUN apt install -y nodejs # from ubuntu16/ubuntu16-install.sh
# RUN apt install -y npm  # from ubuntu16/ubuntu16-install.sh

# We need to have libmysqlclient_r.a to build MySQL-Python statically linked to native MySQL drivers.
# Replace "gcc -dumpversion" with "gcc -dumpfullversion" to fix build on GCC 7.
RUN apt purge --autoremove -y libmysqlclient-dev libmysql++-dev libmysqld-dev && \
    apt install -y libncurses5-dev && \
    curl -L https://downloads.mariadb.org/f/mariadb-5.5.68/source/mariadb-5.5.68.tar.gz | tar xz -C /opt && \
    ln -s /opt/mariadb-5.5.68 /opt/mariadb && \
    cd /opt/mariadb && \
    sed -i "s/-dumpversion/-dumpfullversion/g" ./BUILD/check-cpu && \
    bash ./BUILD/compile-pentium64 && \
    make install
ENV PATH /usr/local/mysql/bin:$PATH

# MHUE-327 Install dependency to build pyodbc
RUN apt install -y unixodbc-dev
