## Installation
### Postfix
#### Installation de Postfix
Installation avec docker:
```bash
docker-compose build postfix
docker-compose up postfix
```
Tester si c'est bien installé:
```bash
docker exec -it postfix bash
#Dans le docker
echo "Body of my mail blah blah blah" | mail -s "subject" root@<your-domain>
```
Verifier que le contenu du mail est bien dans ~/maildir/new

#### Utiliser Thunderbird comme client

Nouveau -> Compte courrier existant

_Nom complet: User1
Adresse électronique: user1@testimt.com
Mot de passe: 1234_

Cliquer sur `Configuration manuelle`

Dans `Server entrant`:

_Protocole: POP3
Nom d'hôte: localhost
Port: 110
Sécurité de la connexion: Aucun
Méthode d'authenficiation: Mot de passe normal
Nom d'utilisateur: user1_

Dans `Serveur sortant`:

_Nom d'hôte: localhost
Port: 25
Sécurité de la connexion: Aucun
Méthode d'authentification: Mot de passe normal
Nom d'utilisateur: user1_

Faire de même avec user2@testimt.com

Tester d'envoyer un mail de user1@testimt.com à user2@testimt.com avec Thunderbird, notamment avec une pièce jointe.