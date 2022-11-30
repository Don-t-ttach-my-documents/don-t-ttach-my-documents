FROM ubuntu:20.04

ENV DEBIAN_FRONTEND non_interactive

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y systemd apache2 nano mailutils gettext
COPY main.cf /etc/postfix/main.cf
COPY mailname /etc/mailname

EXPOSE 25

ADD start.sh /
RUN chmod +x /start.sh
ENTRYPOINT /start.sh