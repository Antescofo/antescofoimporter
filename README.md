# Antescofo Importer

## Deployment

- Increment the version number. Currently it is in `getVersion` method of [ImporterWrapper](ImporterWrapper.cpp) class.
- Document your changes in [build_log](build_log.txt) file.

## Compilation for linux

* Install [Docker](https://download.docker.com/mac/stable/Docker.dmg)
* `make image` to build the docker image to compile
* `make linux` to compile antescofo_importer
* `make testlinux` to execute --help on the newly created binary