SHELL = /bin/sh
top_srcdir = .

INSTALL = /usr/bin/install -c
prefix = /usr/local
exec_prefix = ${prefix}
datarootdir = ${prefix}/share
bindir = ${exec_prefix}/bin
srcdir = .
mandir = ${datarootdir}/man
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
pkgconfigdir = $(libdir)/pkgconfig

mkinstalldirs = $(SHELL) $(srcdir)/mkinstalldirs

CC = gcc
DEFS = -DHAVE_CONFIG_H
CFLAGS = -I. -g -O2
LDFLAGS = 
LIBS =  -lm -ldl
JACK_LIBS = 

AUDIO_LIB = 
JACK_FLAGS = 

LDSO_FLAGS = -shared
SO_FLAGS = -fPIC 
SO_INSTALL = install
A_INSTALL = install
SO_LD = gcc
A_LD = ar
A_LD_FLAGS = -rc
LD_FLAGS = -r
SO_NAME = libsndlib_shuriken.so
RANLIB = :

main_target: sndlib_shuriken

.c.o:
	$(CC) -c $(DEFS) $(CFLAGS) $(SO_FLAGS) $(XEN_CFLAGS) $(GSL_FLAGS) $(JACK_FLAGS) $<

SNDLIB_HEADERS = sndlib.h sndlib-strings.h
SNDLIB_O_FILES = headers.o io.o sound.o
SNDLIB_SIMPLE_O_FILES = headers.o io.o sound.o


sndlib_shuriken: $(SNDLIB_HEADERS) $(SNDLIB_O_FILES)
	$(SO_LD) $(LDFLAGS) $(SNDLIB_O_FILES) -o $(SO_NAME) $(LDSO_FLAGS) $(XEN_LIBS) $(AUDIO_LIB) $(JACK_FLAGS) $(JACK_LIBS) $(LIBS)
	$(A_LD) $(A_LD_FLAGS) libsndlib_shuriken.a $(SNDLIB_O_FILES)
	$(RANLIB) libsndlib_shuriken.a

$(SNDLIB_O_FILES): $(SNDLIB_HEADERS)


clean:
	rm -f $(SNDLIB_O_FILES)

sndplay: $(SNDLIB_HEADERS) $(SNDLIB_O_FILES) sndplay.o
	$(CC) sndplay.c -o sndplay $(SNDLIB_SIMPLE_O_FILES) $(AUDIO_LIB) $(DEFS) $(CFLAGS) $(SO_FLAGS) $(JACK_FLAGS) $(JACK_LIBS) $(LIBS)

sndinfo: $(SNDLIB_HEADERS) $(SNDLIB_O_FILES) sndinfo.o
	$(CC) sndinfo.c -o sndinfo $(SNDLIB_SIMPLE_O_FILES) $(AUDIO_LIB) $(DEFS) $(CFLAGS) $(SO_FLAGS) $(JACK_FLAGS) $(JACK_LIBS) $(LIBS) 

audinfo: $(SNDLIB_HEADERS) $(SNDLIB_O_FILES) audinfo.o
	$(CC) audinfo.c -o audinfo $(SNDLIB_SIMPLE_O_FILES) $(AUDIO_LIB) $(DEFS) $(CFLAGS) $(SO_FLAGS) $(JACK_FLAGS) $(JACK_LIBS) $(LIBS)


Makefile: Makefile.in config.status
	./config.status

config.status: configure
	./config.status --recheck

configure: configure.in
	cd $(srcdir); autoconf

