# Makefile for Linux (use the xproj for Mac!)
# Execute 'make' to create antescofo_importer executable
# Other options:
# make clean
# make debug
#

MAIN = antescofo_importer
CURRENT_DIR = $(shell pwd)
INCLUDES = -Izlib/ -Izlib/unzip/ -Ixml
LIBS = -lpthread -lstdc++ -lm
LFLAGS = -L. zlib/libzlinux.a -static -static-libstdc++
TAG = latest
IMAGENAME = buildimporterlinux

SOURCES = main.cpp ActionGroup.cpp BeatPerMinute.cpp Entry.cpp Event.cpp GeneralMIDISoundset.cpp ImporterWrapper.cpp ImportModel.cpp Measure.cpp MidiImporter.cpp MidiNoteAction.cpp MusicXmlImporter.cpp Nosync.cpp Pitch.cpp rational.cpp Repeat.cpp SimpleRational.cpp Tempo.cpp UnitTester.cpp xml/ConvertUTF.cpp xml/tinystr.cpp xml/tinyxml.cpp xml/tinyxmlerror.cpp xml/tinyxmlparser.cpp zlib/unzip/extract.cpp zlib/unzip/ioapi.cpp zlib/unzip/unzip.cpp
OBJS = $(SOURCES:.cpp=.o)

LEVEL = -O3
EXTRA_DEFINE = -mtune=generic -std=c++11 -ffast-math -fpermissive
CCFLAGS = $(LEVEL) $(EXTRA_DEFINE)
CC = g++
CFLAGS = $(LEVEL) $(EXTRA_DEFINE)

all: main
	@echo Compilation of antescofo_importer completed!

main: $(OBJS)
	$(MAKE) -C zlib -f Makefile_linux
	cd $(CURRENT_DIR)
	$(CC) $(LEVEL) $(CCFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)
	mkdir -p out/linux/
	mv -f $(MAIN) out/linux/

debug:
	$(CC) -g $(CCFLAGS) $(INCLUDES) -o $(MAIN) $(SOURCES) $(LFLAGS) $(LIBS)

clean:
	$(MAKE) -C zlib -f Makefile_linux clean
	rm -rf $(OBJS) *.bin $(MAIN) core *~

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

image:
	cd $(CURRENT_DIR)/Dockerimage && docker build --rm -t $(IMAGENAME):$(TAG) .

linux:
	docker run -v "$$(pwd)":/data $(IMAGENAME):$(TAG) bash -c "make clean && make"

testlinux:
	docker run -ti -v "$$(pwd)"/out/linux:/usr/local/bin $(IMAGENAME):$(TAG) antescofo_importer --help
	docker run -ti -v "$$(pwd)"/out/linux:/data/ $(IMAGENAME):$(TAG) file antescofo_importer

build: image linux