FROM centos:centos7.4.1708
MAINTAINER devops@mapr.com
USER root


RUN rm /etc/yum.repos.d/*
ADD setupfiles/CentOS-Base-Vault.repo /etc/yum.repos.d/
RUN yum clean all


ADD centos7.sh /tmp/centos7.sh
ADD setupfiles /tmp/setupfiles
ADD cross-linux /tmp/cross-linux

RUN chmod -f u+x /tmp/centos7.sh
RUN chmod -f u+x /tmp/cross-linux/*.sh

RUN /tmp/centos7.sh

ENV ANT_HOME /opt/ant
ENV ANT_BIN $ANT_HOME/bin
ENV JAVA_HOME /opt/jdk-1.8.0
ENV JAVA_BIN $JAVA_HOME/bin
ENV M2_HOME /opt/maven
ENV M2_BIN $M2_HOME/bin
ENV NODE_BIN /opt/node/bin

ENV PATH $ANT_BIN:$JAVA_BIN:$M2_BIN:$NODE_BIN:$PATH


RUN mkdir /var/run/sshd
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd
RUN ssh-keygen -f /etc/ssh/ssh_host_rsa_key -N '' -t rsa
RUN ssh-keygen -f /etc/ssh/ssh_host_dsa_key -N '' -t dsa

RUN echo 'root:mapr' | chpasswd


EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
