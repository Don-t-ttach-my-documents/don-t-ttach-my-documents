FROM ubuntu:20.04

ENV DEBIAN_FRONTEND non_interactive

RUN apt-get update --fix-missing
RUN apt-get upgrade -y
RUN apt-get install -y -q  \
    systemd  \
    rsyslog  \
    apache2  \
    nano \
    telnet \
    libmilter-dev \
    libcurl4-openssl-dev \
    mailutils \
    make \
    gcc \
    gettext \
    dovecot-core \
    dovecot-imapd \
    dovecot-pop3d \
    mutt \
    nmap \
    iputils-ping \
    libsasl2-2 \
    ca-certificates \
    libsasl2-modules
COPY main.cf /etc/postfix/main.cf
COPY dovecot.conf /etc/dovecot/dovecot.conf
COPY 10-auth.conf /etc/dovecot/conf.d/10-auth.conf
COPY milter_filter /root/milter_filter
EXPOSE 25 110

RUN useradd -m user1
RUN useradd -m user2
RUN echo "user1:1234" | chpasswd
RUN echo "user2:1234" | chpasswd

COPY sasl_passwd /etc/postfix/sasl_passwd
RUN chmod 400 /etc/postfix/sasl_passwd

WORKDIR /root/milter_filter
RUN make
WORKDIR /

ADD start.sh /
RUN chmod +x /start.sh
RUN chmod +x /var/mail
ENTRYPOINT /start.sh