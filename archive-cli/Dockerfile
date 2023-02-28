FROM ubuntu:latest
# Installer curl
RUN apt-get update
RUN apt-get -y install curl
# Installer mc cli
RUN curl https://dl.min.io/client/mc/release/linux-amd64/mc --create-dirs -o $HOME/minio-binaries/mc
RUN chmod 777 /root/minio-binaries/mc
ENV PATH="$PATH:/root/minio-binaries/"
# Ajouter le script pour supprimer les fichiers
ADD archive_data.sh /root/archive_data.sh
RUN chmod +x /root/archive_data.sh
# Lancer le script
CMD /root/archive_data.sh