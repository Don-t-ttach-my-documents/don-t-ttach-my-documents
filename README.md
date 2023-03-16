## Installation
### Lancer tous les services
Configurer le nom de domaine dans le fichier `.env`, ainsi que les identifiants pour se connecter à MinIO. Puis, lancer les services:

```bash
docker-compose up --build
```

Dans ce fichier, on utilise un nom de domaine `test.com`

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
echo "Body of my mail blah blah blah" | mail -s "subject" root@test.com
```
Verifier que le contenu du mail est bien dans ~/maildir/new

#### Utiliser Thunderbird comme client

Nouveau -> Compte courrier existant

- _Nom complet: User1_
- _Adresse électronique: user1@test.com_
- _Mot de passe: 1234_

Cliquer sur `Configuration manuelle`

Dans `Server entrant`:

- _Protocole: POP3_
- _Nom d'hôte: localhost_
- _Port: 110_
- _Sécurité de la connexion: Aucun_
- _Méthode d'authenficiation: Mot de passe normal_
- _Nom d'utilisateur: user1_

Dans `Serveur sortant`:

- _Nom d'hôte: localhost_
- _Port: 587_
- _Sécurité de la connexion: Aucun_
- _Méthode d'authentification: Mot de passe normal_
- _Nom d'utilisateur: user1_

Faire de même avec user2@test.com

Tester d'envoyer un mail de user1@test.com à user2@test.com avec Thunderbird, notamment avec une pièce jointe.
