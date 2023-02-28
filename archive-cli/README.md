# Setup du cronjob
Pour mettre en place le service d'archive, cron doit être installé sur la machine.

Inclure le cronjob dans la liste des tâches cron en prenant le soin d'indiquer le chemin vers le [docker-compose](/docker-compose.yml) de la racine, et en changeant selon les besoins, les délais d'éxecution de la tâche (voir documentation de cron) dans le fichier [basic.cronjob](/archive-cli/basic.cronjob).

Le cronjob va se contenter de lancer le service archive-cli du [docker-compose](/docker-compose.yml). Ce service va exécuter le script [archive_data](/archive-cli/archive_data.sh), ayant pour but de supprimer tous les fichiers datant de plus de *Delay*. Ce délai peut être changé dans le [docker-compose](/docker-compose.yml).