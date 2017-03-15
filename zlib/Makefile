# Makefile for zlib
# Copyright (C) 1995-2010 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h

# To compile and test, type:
#    ./configure; make test
# Normally configure builds both a static and a shared library.
# If you want to build just a static library, use: ./configure --static

# To use the asm code, type:
#    cp contrib/asm?86/match.S ./match.S
#    make LOC=-DASMV OBJA=match.o

# To install /usr/local/lib/libz.* and /usr/local/include/zlib.h, type:
#    make install
# To install in $HOME instead of /usr/local, use:
#    make install prefix=$HOME

CC=clang

CFLAGS=-O3 -DNO_FSEEKO -DNO_snprintf -DHAS_sprintf_void -DNO_VIZ -arch i386 -arch x86_64

SFLAGS=-O3 -fPIC -DNO_FSEEKO -DNO_snprintf -DHAS_sprintf_void -DNO_VIZ
LDFLAGS= -L. libz.a
TEST_LDFLAGS=-L. libz.a
LDSHARED=gcc
CPP=gcc -E

STATICLIB=libz.a
SHAREDLIB=
SHAREDLIBV=
SHAREDLIBM=
LIBS=$(STATICLIB) $(SHAREDLIBV)

AR=ar rc
RANLIB=ranlib
LDCONFIG=ldconfig
LDSHAREDLIBC=-lc
TAR=tar
SHELL=/bin/sh
EXE=

prefix =/usr/local
exec_prefix =${prefix}
libdir =${exec_prefix}/lib
sharedlibdir =${libdir}
includedir =${prefix}/include
mandir =${prefix}/share/man
man3dir = ${mandir}/man3
pkgconfigdir = ${libdir}/pkgconfig

OBJC = adler32.o compress.o crc32.o deflate.o gzclose.o gzlib.o gzread.o \
	gzwrite.o infback.o inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o

OBJA =
PIC_OBJA =

OBJS = $(OBJC) $(OBJA)

all: libz.a

libz.a: $(OBJS)
	$(AR) $@ $(OBJS)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

clean:
	rm -f *.o *.lo *~ \
	   example$(EXE) minigzip$(EXE) examplesh$(EXE) minigzipsh$(EXE) \
	   example64$(EXE) minigzip64$(EXE) \
	   libz.* foo.gz so_locations \
	   _match.s maketree contrib/infback9/*.o
	rm -rf objs
