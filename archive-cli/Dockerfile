FROM ubuntu:latest
# Install cron
RUN apt-get update
RUN apt-get -y install curl
# Install mc cli
RUN curl https://dl.min.io/client/mc/release/linux-amd64/mc --create-dirs -o $HOME/minio-binaries/mc
RUN chmod 777 /root/minio-binaries/mc
ENV PATH="$PATH:/root/minio-binaries/"
# Add script for removal
ADD archive_data.sh /root/archive_data.sh
RUN chmod +x /root/archive_data.sh
# Launch script
CMD /root/archive_data.sh