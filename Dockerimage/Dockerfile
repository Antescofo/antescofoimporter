FROM --platform=linux/amd64 debian:11

RUN apt-get update && apt-get install --no-install-recommends -y make gcc build-essential file && rm -rf /var/lib/apt/lists/*
WORKDIR /data
CMD [ "/bin/bash" ]
