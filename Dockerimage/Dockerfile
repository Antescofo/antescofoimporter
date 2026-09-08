FROM debian:bookworm

RUN apt-get update && apt-get install --no-install-recommends -y build-essential file && rm -rf /var/lib/apt/lists/*
WORKDIR /data
CMD [ "/bin/bash" ]
