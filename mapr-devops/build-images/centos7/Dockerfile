FROM maprdocker.lab/centos7-java8-build:latest
MAINTAINER devops@mapr.com

RUN yum clean all
RUN yum install -y libffi-devel gmp-devel openldap-devel libxml2-devel libxslt-devel sqlite-devel

COPY MariaDB.repo /etc/yum.repos.d/MariaDB.repo
RUN yum clean all
RUN yum install -y mariadb-libs

RUN ln -s /opt/maven/bin/mvn /usr/local/bin/mvn

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]