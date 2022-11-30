#!/bin/bash

cat etc/mailname | envsubst '$HOSTNAME' > etc/mainame
cat etc/postfix/main.cf | envsubst '$HOSTNAME' > etc/postfix/main.cf
service apache2 restart
service postfix restart
/bin/bash