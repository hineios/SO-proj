PROGRAMS = cache-lock

COMPILE = $(CC) $(CFLAGS)
CC = gcc
CFLAGS = -g -O0 -Wall -m32

all: $(PROGRAMS)

%:	%.c
	$(COMPILE)  -o $@ $< 

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
