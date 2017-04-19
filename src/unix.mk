# D-ITG Main Makefile

###################
# Dynamic Options #
###################

# Enable SCTP support if specified on command line (requires kernel version >= 2.5.15)
ifdef sctp
	CXXOPT += -DSCTP
	LDOPT  += -lsctp
endif

# Enable DCCP support if specified on command line (requires kernel version >= 2.6.14-rc1)
ifdef dccp
	CXXOPT += -DDCCP
endif

# Set debug level as specified on command line (debug level may be increased to have more information)
ifdef debug
        CXXOPT += -DDEBUG=$(debug) -O0 -ggdb
else
        CXXOPT += -O2 
endif

# Disable bursty flows if specified on command line.
ifneq ($(bursty),off) 
	CXXOPT += -DBURSTY
endif

# Disable support for receiving multiple data flows on the same listening port if specified on command line.
ifneq ($(multiport),off)
	CXXOPT += -DMULTIPORT
endif

##################
# Static options # 
##################

# Locals 
VERSION=$(shell cat ../VERSION)
REVISION=$(shell [ -d ../.svn ] && svnversion -n || cat ../REVISION)
RELEASE += -DVERSION="\"$(VERSION)\""
RELEASE += -DREVISION="\"$(REVISION)\""

COMPONENTS = ITGSend ITGRecv ITGLog ITGDec libITG ITGManager
WARNINGS = -Wall -Wno-deprecated -Wno-strict-aliasing
PREFIX = /usr/local

# OS dependent options
OS := $(shell uname)
ifeq ($(OS),Linux)
	OSFLAGS := -DUNIX
endif
ifeq ($(OS),FreeBSD)
	OSFLAGS := -DUNIX -DBSD
endif
ifeq ($(OS),Darwin)
	OSFLAGS := -DUNIX -DBSD -DOSX
	# Warning! At the moment D-ITG has no support for IPv6 on Apple OSX
	CXXOPT += -DNOIPV6
endif

# gcc version dependent options
GCC_VERSION = $(shell gcc -dumpversion | cut -f1 -d.)
GCC_MAJOR = $(shell gcc -dumpversion | cut -f2 -d.)
GCC_MINOR = $(shell gcc -dumpversion | cut -f3 -d.)
ifeq ($(GCC_VERSION),4)
    ifeq ($(shell expr $(GCC_MAJOR) \>= 4),1)
	WARNINGS += -Wno-unused-result
    endif
endif

# Global options
export OS
export RM = rm -f
export CP = cp
export MV = mv
export SUFFIX =
export SOSUFFIX = .so
export CXXFLAGS = $(CXXOPT) $(OSFLAGS) $(RELEASE) $(WARNINGS) -fPIC
export LDFLAGS = -lpthread -lm $(LDOPT)
export BASEDIR = $(shell pwd)
export BIN = $(shell dirname $(BASEDIR))/bin
export COMMON = $(BASEDIR)/common/
export NRCLEAN = $(BASEDIR)/ITGSend/newran/
export EXEC_DIR = /usr/local/bin
export CXX = g++
export RANLIB = ranlib
export THOBJS = common/thread.o
export OBJS = common/ITG.o common/timestamp.o common/serial.o common/pipes.o

#################
# Generic Rules #
#################

.PHONY: $(COMPONENTS)
all: check head $(COMPONENTS)
	@ printf '\n----------------------------------------------------------\n'
	@ echo 'D-ITG executables created in $(BIN)'

#####
help:
	@ echo '----------------'
	@ echo 'Building options'
	@ echo '----------------'
	@ echo '   debug=<N>      : enable debug mode using verbosity level N'
	@ echo '   multiport=off  : disable support for receiving multiple flows on the same port number' 
	@ echo '   bursty=off     : disable support for bursty flows' 
ifeq ($(OS),Linux)
	@ echo '   dccp=on        : enable DCCP transport protocol support'
	@ echo '   sctp=on        : enable SCTP transport protocol support'
	@ echo '   T=win          : cross-compile binaries for Windows'
endif 
	@ echo '-------------'
	@ echo 'Building tips'
	@ echo '-------------'
	@ echo ' - To build D-ITG with options use the following syntax:' 
	@ echo '    $(MAKE) clean all [options]'
	
#####
head:
	@ printf '\n---------------------\n'
	@ echo 'Current configuration'
	@ echo '---------------------'
	@ printf 'Enabled features*: '
ifdef debug
	@ printf 'debug=$(debug) '
endif
ifdef dccp
	@ printf 'dccp '
endif
ifdef sctp
	@ printf 'sctp '
endif
ifneq ($(bursty),off)
	@ printf 'bursty '
endif
ifneq ($(multiport),off)
	@ printf 'multiport '
endif
	@ echo 
	@ echo 'Version: $(VERSION)'
	@ echo 'Revision: $(REVISION)'
	@ echo 'Compiler: $(CXX)'
	@ echo 'OS: $(OS)'
	@ echo 'OS Flags: $(OSFLAGS)'
	@ echo '------------------------------------------------'
	@ echo '* Run "$(MAKE) help" for available options and tips'
	@ printf '\n---------------------\n'
	@ echo 'Building common files'
	@ echo '---------------------'

######
check:
	@ printf '\n---------------------\n'
	@ echo 'Checking dependencies'
	@ echo '---------------------'
ifdef dccp
    ifneq ($(OS),Linux)
	@ echo 'DCCP protocol currently supported only on Linux.'
	@ exit 1
    endif
endif
ifdef sctp
    ifeq ($(OS),Linux)
	@ printf 'SCTP development library...'
	@ [ -e /usr/include/netinet/sctp.h -o -e /usr/local/include/netinet/sctp.h ] \
		&& { echo 'found' ; } \
		|| { printf 'not found.\nPlease install SCTP development libraries and try again.\n' ; exit 1 ; }
    else
	@ echo 'SCTP protocol currently supported only on Linux.'
	@ exit 1
    endif
endif
	@ echo 'All dependencies satisfied.'
	
##############
$(COMPONENTS): $(THOBJS) $(OBJS)
	@ printf '\n-------------------\n'
	@ echo 'Building $@'
	@ echo '-------------------'
	@ $(MAKE) -C $@ --no-print-directory

###########
check_ditg:
	@ [ -e $(PREFIX)/bin/ITGSend ] || { echo 'D-ITG is not installed.' ; exit 1 ; }

##########
check_uid:
	@ [ `id -u` -eq 0 ] || { echo 'You need to be root to install D-ITG.' ; exit 1; }
	
########
install: check_uid
	@ echo '----------------'
	@ echo 'Installing D-ITG'
	@ echo '----------------'
	@ mkdir -p "$(PREFIX)/bin"
	@ for e in ITGSend ITGRecv ITGLog ITGDec ITGManager; do \
		echo -n "Copying $${e}..." ;\
		$(CP) $(BIN)/$${e}$(SUFFIX) "$(PREFIX)/bin" ;\
		echo 'done' ;\
	  done
	@ mkdir -p "$(PREFIX)/lib"
	@ printf "Copying libITG$(SOSUFFIX)..."
	@ $(CP) $(BIN)/libITG$(SOSUFFIX) "$(PREFIX)/lib"
	@ echo 'done'
	@ printf '\n----------------------------------------------------------\n'
	@ echo 'D-ITG installed in $(BIN)'
	
##########
uninstall: check_uid check_ditg
	@ echo '------------------'
	@ echo 'Uninstalling D-ITG'
	@ echo '------------------'
	@ printf "Removing components..."
	@ $(RM) $(PREFIX)/bin/ITG*$(SUFFIX)
	@ echo 'done'
	@ printf "Removing libITG$(SOSUFFIX)..."
	@ $(RM) $(PREFIX)/lib/libITG$(SOSUFFIX)
	@ echo 'done'
	
######
clean: 
	@ echo '----------------'
	@ echo 'Cleaning sources'
	@ echo '----------------'
	@ printf 'Cleaning common files...'
	@ $(RM) $(COMMON)/*.o
	@ echo 'done'
	@ for c in ITGSend ITGRecv ITGLog ITGDec libITG ITGManager; do \
		printf "Cleaning $${c}..." ;\
		$(MAKE) -C $${c} --no-print-directory clean ;\
		echo 'done' ;\
	  done
	  
#---------------
include common.mk
#---------------

# EOF
