#####
#
# File storage management, interfacing with Minio
#
#####

import jwt
import requests
from minio import Minio
from urllib.parse import urlparse
import os

from variables import MINIO_HOST, MINIO_USER, MINIO_PASSWORD, BUCKET_NAME, SECRET

client = Minio(MINIO_HOST, access_key=MINIO_USER, secret_key=MINIO_PASSWORD, secure=False)


def upload_to_minio(file, email):
    check_bucket_exists()

    size = os.fstat(file.fileno()).st_size
    client.put_object(BUCKET_NAME, email + "/" + file.filename, file, size)
    return build_url(email, file.filename)


def build_url(email, filename):
    url = client.presigned_get_object(BUCKET_NAME, email + "/" + filename)
    query = str(urlparse(url).query)
    token = jwt.encode({"query": query}, SECRET, algorithm="HS256")
    res_url = "/file/" + filename + "?sender=" + email + "&token=" + token
    return res_url


def get_file(email, filename, infos):
    file_url = "https://" + MINIO_HOST + "/" + BUCKET_NAME + "/" + email + "/" + filename + "?" + infos["query"]
    return requests.get(file_url)


def check_bucket_exists():
    found = client.bucket_exists(BUCKET_NAME)
    if not found:
        client.make_bucket(BUCKET_NAME)
