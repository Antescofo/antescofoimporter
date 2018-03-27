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

SOURCES = main.cpp ImporterWrapper.cpp rational.cpp ImportModel.cpp UnitTester.cpp Event.cpp Measure.cpp Entry.cpp Pitch.cpp Repeat.cpp Tempo.cpp BeatPerMinute.cpp MusicXmlImporter.cpp MidiImporter.cpp MidiNoteAction.cpp ActionGroup.cpp GeneralMIDISoundset.cpp SimpleRational.cpp xml/ConvertUTF.cpp xml/tinystr.cpp xml/tinyxml.cpp xml/tinyxmlerror.cpp xml/tinyxmlparser.cpp zlib/unzip/unzip.cpp zlib/unzip/ioapi.cpp zlib/unzip/extract.cpp
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
	docker run -ti -v "$$(pwd)"/out/linux:/usr/local/bin alpine antescofo_importer --help

