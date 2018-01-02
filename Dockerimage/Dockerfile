FROM debian:8

RUN apt-get update && apt-get install --no-install-recommends -y make gcc build-essential && rm -rf /var/lib/apt/lists/*
WORKDIR /data
CMD [ "/bin/bash" ]
