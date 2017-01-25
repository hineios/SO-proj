
SUBDIRS =  sthread_lib logrw sofs

#
# Dados sobre o grupo e turno frequentado 
# CAMPUS = preencher com A ou T consoante Alameda ou Tagus
# CURSO = indicar o curso do turno frequentado: LEIC ou LERC
# GRUPO = indicar o numero do grupo
# ALUNO1/ALUNO2/ALUNO3 = indicar os numeros dos alunos
#
CAMPUS=ALAMEDA
CURSO=LEIC
GRUPO=52
ALUNO1=70031
ALUNO2=70137
ALUNO3=70227

TOPO=nocode

#LBITS := $(shell getconf LONG_BIT)
#ifeq ($(LBITS),64)
#  ARCH=-D_FILE_OFFSET_BITS=64
#else
#  ARCH=
#endif

ifeq ($(TOPO),nocode)
DEFS= -m32 -DSLEEP_ON -DHAVE_CONFIG_H -DUSE_PTHREADS -DSIMULATE_IO_DELAY -DNOT_FS_INITIALIZER -D_FILE_OFFSET_BITS=64
USE_PTHREADS=1
export USE_PTHREADS
endif
ifeq ($(TOPO),logfs)
DEFS= -m32 -DUSE_LOG_SOFS -DUSE_PTHREADS -DSLEEP_ON -DHAVE_CONFIG_H -DSIMULATE_IO_DELAY -DNOT_FS_INITIALIZER -D_FILE_OFFSET_BITS=64
USE_PTHREADS=1
export USE_PTHREADS
endif
ifeq ($(TOPO),sthreads)
DEFS= -m32 -DSLEEP_ON -DHAVE_CONFIG_H -DSIMULATE_IO_DELAY -DNOT_FS_INITIALIZER -D_FILE_OFFSET_BITS=64 
endif
ifeq ($(TOPO),sthlogs)
DEFS= -m32 -DUSE_LOG_SOFS -DSLEEP_ON -DHAVE_CONFIG_H -DSIMULATE_IO_DELAY -DNOT_FS_INITIALIZER -D_FILE_OFFSET_BITS=64
endif
ifeq ($(TOPO),cache)
DEFS= -m32 -DUSE_CACHE -DUSE_PTHREADS -DSLEEP_ON -DHAVE_CONFIG_H -DSIMULATE_IO_DELAY -DNOT_FS_INITIALIZER -D_FILE_OFFSET_BITS=64
USE_PTHREADS=1
export USE_PTHREADS
endif
ifeq ($(TOPO),logfscache)
DEFS= -m32 -DUSE_CACHE -DUSE_LOG_SOFS -DUSE_PTHREADS -DSLEEP_ON -DHAVE_CONFIG_H -DSIMULATE_IO_DELAY -DNOT_FS_INITIALIZER -D_FILE_OFFSET_BITS=64
USE_PTHREADS=1
export USE_PTHREADS
endif

CFLAGS = -g -O0 -Wall
export DEFS
export CFLAGS

all: build

build:
	@list='$(SUBDIRS)'; for p in $$list; do \
	  echo "Building $$p"; \
	  $(MAKE) -C $$p; \
	done

clean:
	@list='$(SUBDIRS)'; for p in $$list; do \
	  echo "Cleaning $$p"; \
	  $(MAKE) clean -C $$p; \
	done

package: clean zip

zip:
ifndef CAMPUS
	@echo "ERROR: Must setup macro 'CAMPUS' correcly."
else
ifndef CURSO
	@echo "ERROR: Must setup macro 'CURSO' correcly."
else
ifndef GRUPO
	@echo "ERROR: Must setup macro 'GRUPO' correcly."
else
	tar -czf project-$(CAMPUS)-$(CURSO)-$(GRUPO)-$(ALUNO1)-$(ALUNO2)-$(ALUNO3).tgz * 
endif
endif
endif
