PROGRAMS = sched_1cl3ths sched_3cl_6th sched_dumptest test-ReadWrite test-time-slice  

INCLUDES = -I. -I$(srcdir) -I../include -I ../include
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)
CC = gcc
CFLAGS = -g -O0 -Wall -m32 -pthread 
DEFS = -DHAVE_CONFIG_H -DUSE_PTHREADS
LIBSTHREAD = ../sthread_lib/libsthread.a

all: $(PROGRAMS)

%:	%.c
	$(COMPILE) ../sthread_lib/sthread_start.o -o $@ $< $(LIBSTHREAD)

clean: clean-PROGRAMS
	rm -f *.o

clean-PROGRAMS:
	@list='$(PROGRAMS)'; for p in $$list; do \
	  echo " rm -f $$p $$p.o"; \
	  rm -f $$p $$p.o ; \
	done


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
