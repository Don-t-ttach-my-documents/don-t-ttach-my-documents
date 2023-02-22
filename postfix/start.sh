#!/bin/bash

# Remplace les variables d'environnement dans les fichiers dans lesquelles on en a besoin
replace_env_in_file () {
  cat $1 | envsubst "`printf '${%s} ' $(sh -c "env|cut -d'=' -f1")`" > temp
  mv temp $1
}

replace_env_in_file etc/postfix/main.cf

#Démarrage des services
cd /root/milter_filter
./myFilter -p inet:8800@localhost &
service rsyslog restart
service apache2 restart
service postfix restart
service dovecot restart
/bin/bash