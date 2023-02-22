import os
import sys

if os.getenv("DOMAIN") is None:
    sys.exit()

DOMAIN = os.getenv("DOMAIN")