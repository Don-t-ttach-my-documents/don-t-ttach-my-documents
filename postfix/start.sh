#!/bin/bash

# Remplace les variables d'environnement dans les fichiers dans lesquelles on en a besoin
replace_env_in_file () {
  cat $1 | envsubst "`printf '${%s} ' $(sh -c "env|cut -d'=' -f1")`" > temp
  mv temp $1
}

replace_env_in_file etc/postfix/main.cf
postmap /etc/postfix/sasl_passwd

cd etc/ssl/certs && openssl req -newkey rsa:2048 \
      -new -nodes -x509 -days 3650 \
      -keyout key-for-smtp-gmail.pem \
      -out cert-for-smtp-gmail.pem \
      -subj '/CN=$HOSTNAME/O=IMT Atlantique/C=FR'
cp /etc/ssl/certs/cert-for-smtp-gmail.pem /etc/postfix/cacert.pem

#Démarrage des services
cd /root/milter_filter
./myFilter -p inet:8800@localhost &
service rsyslog restart
service apache2 restart
service postfix restart
service dovecot restart
/bin/bash