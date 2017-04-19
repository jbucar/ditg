# D-ITG Linux-MinGW Main Makefile

###################
# Dynamic Options #
###################

# Switch to IPv6 mode if specified on command line
ifndef ipv4only	
	CXXOPT += -DIPv6RECV
endif		

# Set debug level as specified on command line (debug level may be increased to have more information)
ifdef debug
        CXXOPT += -DDEBUG=$(debug) -ggdb
else
        CXXOPT += -O2 
endif

# Disable multiport if specified on command line.
ifdef multiport 
        CXXOPT += -DMULTIPORT
endif

# Enable static compilation
ifdef static
	CXXOPT += -static-libstdc++ -static-libgcc
endif

##################
# Static options # 
##################

VERSION=$(shell cat ../VERSION)
REVISION=$(shell [ -d ../.svn ] && svnversion -n || cat ../REVISION)

CXXOPT += -DVERSION="\"$(VERSION)\""
CXXOPT += -DREVISION="\"$(REVISION)\""

# Locals
COMPONENTS = ITGSend ITGRecv ITGLog ITGDec libITG ITGManager

# Globals
export OS       = WindowsNT
export OSFLAGS  = -DWIN32
export SUFFIX   = .exe
export SOSUFFIX = .dll

export RM = rm -rf
export CP = cp
export MV = mv

export BASEDIR = $(shell pwd)
export BIN     = $(shell dirname $(BASEDIR))/bin
export COMMON  = $(BASEDIR)/common/
export NRCLEAN = $(BASEDIR)/ITGSend/newran/

export THOBJS = common/thread.o
export OBJS   = common/ITG.o common/timestamp.o common/serial.o common/pipes.o

#################
# Generic Rules #
#################

.PHONY: $(COMPONENTS)
all: check head $(COMPONENTS) 
	@ echo -e '\n----------------------------------------------------------'
ifdef strip
	@ echo -n 'Stripping binaries...'
	@ $(PREFIX)-strip ../bin/*.exe ../bin/*.dll
	@ echo 'done'
endif
	@ echo 'D-ITG executables created in $(BIN)'

#####
help:
	@ echo '----------------'
	@ echo 'Building options'
	@ echo '----------------'
	@ echo '   debug=<N>      : enable debug mode using verbosity level N'
	@ echo '   ipv4only=on    : disable support for IPv6 (only required on Windows XP to support IPv4)'
	@ echo '   multiport=on   : enable support for receiving multiple flows on the same port number' 
	@ echo '   static=on      : statically include libstdc++ and libgcc' 
	@ echo '-------------'
	@ echo 'Building tips'
	@ echo '-------------'
	@ echo ' - To build D-ITG with options use the following syntax:' 
	@ echo '    make -f mingw.mk clean all [options]'

#####
head:
	@ echo -e '\n---------------------'
	@ echo 'Current configuration'
	@ echo '---------------------'
	@ echo -n 'Enabled features*: '
ifdef ipv4only
	@ echo -n 'ipv4only '
endif
ifdef multiport
	@ echo -n 'multiport ' 
endif
ifdef static
	@ echo -n 'static ' 
endif
ifdef debug
	@ echo -n 'debug=$(debug) '
endif
	@ echo 
	@ echo 'Version: $(VERSION)'
	@ echo 'Revision: $(REVISION)'
	@ echo 'Compiler: $(CXX)'
	@ echo 'OS: $(OS)'	
	@ echo 'OS Flags: $(OSFLAGS)'
	@ echo '------------------------------------------------'
	@ echo '* Run "make help" for available options and tips'
	@ echo -e '\n---------------------'
	@ echo 'Building common files'
	@ echo '---------------------'

######
check:
	@ echo -e '\n---------------------'
	@ echo 'Checking dependencies'
	@ echo '---------------------'
ifdef MINGW
	@ $(eval PREFIX := $(shell echo "$(MINGW)" | awk -F/ '{ print $$NF ; exit }'))
	@ $(eval GWDIR  := $(MINGW))
else
	@ echo 'Available MinGW environments:'
	@ find /usr -maxdepth 1 -name '*mingw*' | \
		awk '{ if ($$0 ~ /x86_64/) { a="64bit" } else { a="32bit" } ; print "- " $$0 " ("a")"}' || echo "not found!"
	@ echo 'Note: *** only 32bit targets are currently supported ***'
	@ $(eval PREFIX := $(shell find /usr -maxdepth 1 -name '*mingw*' | awk -F/ '!/x86_64/{ print $$NF ; exit }'))
	@ $(eval GWDIR  := /usr/$(PREFIX))
endif
	@ if [ $(PREFIX) ]; then \
		echo 'Selected MinGW environment: $(GWDIR)' ;\
	 else \
		echo 'Please set the MINGW variable to point to the MinGW toolchain path (e.g. MINGW=/usr/i586-mingw32msvc)' ;\
		exit 1 ;\
	 fi
	@ $(eval export LDFLAGS = $(GWDIR)/lib/libws2_32.a)
	@ $(eval CXXINCS  = -I"$(GWDIR)/include")
	@ $(eval LIBS     = -L"$(GWDIR)/lib")
	@ $(eval INCS     = -I"$(GWDIR)/include")
	@ $(eval CFLAGS   = $(INCS))
	@ $(eval export CXXFLAGS = $(CXXOPT) -Wall $(OSFLAGS) -D_WIN32_WINNT=0x0501 -Wno-deprecated -Wno-strict-aliasing  $(CXXINCS))
	@ $(eval export CXX      = $(PREFIX)-c++)
	@ $(eval export CC       = $(PREFIX)-gcc)
	@ $(eval export AR       = $(PREFIX)-ar)
	@ $(eval export RANLIB   = $(PREFIX)-ranlib)
	@ echo -n 'Compiler: '
	@ which $(CXX) || { echo "$(CXX) not in the PATH!" ; exit 1 ; }
	@ echo '(To select a different environment set the MINGW variable (e.g. MINGW=/usr/i586-mingw32msvc)' 

######
clean: 
	@ echo '----------------'
	@ echo 'Cleaning sources'
	@ echo '----------------'
	@ echo -n 'Cleaning common files...'
	@ $(RM) $(COMMON)/*.o
	@ echo 'done'
	@ for c in ITGSend ITGRecv ITGLog ITGDec libITG ITGManager; do \
		echo -n "Cleaning $${c}..." ;\
		make -C $${c} --no-print-directory clean ;\
		echo 'done' ;\
	  done

$(COMPONENTS): $(THOBJS) $(OBJS)
	@ echo -e '\n-------------------'
	@ echo 'Building $@'
	@ echo '-------------------'
	@ $(MAKE) -C $@ --no-print-directory

#---------------
include common.mk
#---------------

# EOF
