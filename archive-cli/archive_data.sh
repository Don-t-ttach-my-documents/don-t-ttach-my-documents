mc alias set filestorage http://minio:9000 remi_czn password
mc rm filestorage -older-than 30d --recursive --force --dangerous --fake