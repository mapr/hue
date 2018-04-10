FROM centos:centos6.6
MAINTAINER devops@mapr.com
USER root


RUN rm /etc/yum.repos.d/*
ADD setupfiles/CentOS-Base-Vault.repo /etc/yum.repos.d/
RUN yum clean all


ADD centos6.sh /tmp/centos6.sh
ADD setupfiles /tmp/setupfiles
ADD cross-linux /tmp/cross-linux

RUN chmod -f u+x /tmp/centos6.sh
RUN chmod -f u+x /tmp/cross-linux/*.sh

RUN /tmp/centos6.sh


RUN mkdir /var/run/sshd
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd
RUN ssh-keygen -f /etc/ssh/ssh_host_rsa_key -N '' -t rsa
RUN ssh-keygen -f /etc/ssh/ssh_host_dsa_key -N '' -t dsa

RUN echo 'root:mapr' | chpasswd


EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
