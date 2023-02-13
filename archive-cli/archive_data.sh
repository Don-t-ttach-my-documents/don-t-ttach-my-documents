mc alias set filestorage http://minio:9000 $MINIO_ROOT_USER $MINIO_ROOT_PASSWORD
mc rm filestorage -older-than $DELAY --recursive --force --dangerous --fake