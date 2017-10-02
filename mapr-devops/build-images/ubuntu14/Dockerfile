FROM maprdocker.lab/ubuntu14-java8-build:latest
MAINTAINER devops@mapr.com

RUN apt-get update
RUN apt-get install -y libffi-dev libgmp3-dev libldap2-dev libxml2-dev libxslt-dev libsqlite3-dev libsasl2-dev

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]