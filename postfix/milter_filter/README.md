## Modification de l'adresse d'envoi du mail au service de parsing
La ligne 62 du fichier [parsing.c](/postfix/milter_filter/src/parsing.c) contient dans la variable url, le lien vers le service de parsing. Changer cette adresse pour pointer vers la bonne adresse du service de parsing avant de compiler

## Compilation du filtre
```bash
make
```
## Lancement du filtre
```bash
./myFilter -p inet:8800@localhost #inet:port@host
```
# Connexion avec un serveur mail
Voir la documentation pour plus de détails.
La documentation pour Postfix est à cette [adresse](https://www.postfix.org/MILTER_README.html)