# Antescofo Importer

## Deployment

- Increment the version number. Currently it is in `getVersion` method of [ImporterWrapper](ImporterWrapper.cpp) class.
- Document your changes in [build_log](build_log.txt) file.

## Compilation for linux

* Install [Docker](https://download.docker.com/mac/stable/Docker.dmg).
* Run `make build` to build the Docker image, then compile antescofo_importer.
* Run `make testlinux` to execute --help on the newly created binary and make sure that the binary is compiled for x86-64.