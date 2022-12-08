import os
import sys

if os.getenv("MINIO_ROOT_PASSWORD") is None or os.getenv("MINIO_ROOT_USER") is None:
    sys.exit()

BUCKET_NAME = "don-t-ttach-my-docs"
SECRET = "secret"
MINIO_HOST = "minio:9000"

MINIO_USER = os.getenv("MINIO_ROOT_USER")
MINIO_PASSWORD = os.getenv("MINIO_ROOT_PASSWORD")