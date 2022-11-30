## Installation
### Postfix
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