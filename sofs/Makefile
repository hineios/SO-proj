PROGRAMS = sofs 
INCLUDES = -I. -I../include
COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)
CC = gcc
CFLAGS = -g  -O0 -Wall -std=c99
#DEFS ?=-DHAVE_CONFIG_H -DSIMULATE_IO_DELAY -DSLEEP_ON -DNOT_FS_INITIALIZER
#DEFS ?= -DHAVE_CONFIG_H -DSIMULATE_IO_DELAY -DNOT_FS_INITIALIZER  
#DEFS ?=  -DUSE_CACHE -DSLEEP_ON -DHAVE_CONFIG_H -DSIMULATE_IO_DELAY -DNOT_FS_INITIALIZER  
DEFS ?=  -DUSE_CACHE -DUSE_LOG_SOFS -DUSE_PTHREADS -DSLEEP_ON -DHAVE_CONFIG_H -DSIMULATE_IO_DELAY -DNOT_FS_INITIALIZER -DNOT_FS_INITIAL -D_FILE_OFFSET_BITS=64
LIBSTHREAD = ../sthread_lib/libsthread.a 
LIBSLOGRW = ../logrw/liblogrw.a 
LIBSOCKS =  -lpthread 
OBJECTS = fs.o cache.o 

all: libs $(PROGRAMS)

.SUFFIXES: .c .o

sofs: sofs.o $(OBJECTS)
	gcc -m32 -Wall ../sthread_lib/sthread_start.o $(shell pkg-config fuse --libs)-o sofs sofs.o $(OBJECTS) $(LIBSTHREAD) $(LIBSOCKS) $(LIBSLOGRW)

libs:
	$(MAKE) libsthread.a -C ../sthread_lib

sofs.o: sofs.c
	gcc $(DEFS) -g -lfuse -c sofs.c
	#gcc $(DEFS) -g $(shell pkg-config fuse --cflags) -c sofs.c

.c.o:
	$(COMPILE) $(shell pkg-config fuse --cflags) -c -o $@ $<

clean: clean-PROGRAMS
	rm -f *.o

clean-PROGRAMS:
	@list='$(PROGRAMS)'; for p in $$list; do \
	  f=`echo $$p|sed 's/$$//'`; \
	  echo " rm -f $$p $$f"; \
	  rm -f $$p $$f ; \
	done

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:

