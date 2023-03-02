# Parsing smtp message
Pour tester le service, lancer
```bash
python -m venv venv/
. ./venv/Scripts/activate
pip install -r requirements.txt
python parsing.py test/smtp_test.txt
```
avec *smtp_test.txt* le fichier contenant le message smtp

# Lancer le service
```bash
python -m venv venv/
. ./venv/Scripts/activate
pip install -r requirements.txt
python index.py
```