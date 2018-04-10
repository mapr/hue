FROM ubuntu:14.04
MAINTAINER devops@mapr.com
USER root


ADD ubuntu14.sh /tmp/ubuntu14.sh
ADD setupfiles /tmp/setupfiles
ADD cross-linux /tmp/cross-linux

RUN chmod -f u+x /tmp/ubuntu14.sh
RUN chmod -f u+x /tmp/cross-linux/*.sh

RUN /tmp/ubuntu14.sh


RUN mkdir /var/run/sshd
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

RUN echo 'root:mapr' | chpasswd


EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
