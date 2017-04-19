##########################################################################
# Uncomment the following lines to enable options when using Dev-Cpp IDE #
##########################################################################

#ipv4only=on	# Disable support for IPv6 (only required on Windows XP to support IPv4)
#debug=1        # Enable debug mode (debug level may be increased to have more information)
#multiport=on	# Enable support for receiving multiple data flows on the same listening port

##########################################################################
# Check that following line points to the Dev-Cpp installation directory #
##########################################################################

DEVDIR = $(SystemDrive)/dev-cpp

########################################################
# Touching the following lines should not be necessary #
########################################################

#---------------------
# Command-line options
#---------------------

# Enable support for receiving multiple data flows on the same listening port
ifdef multiport
       CXXOPT += -DMULTIPORT
endif

# Disable support for IPv6 (only required on Windows XP to support IPv4)
ifndef ipv4only	
	CXXOPT += -DIPv6RECV
endif		

# Set debug output level
ifdef debug
	CXXOPT += -DDEBUG=$(debug) -ggdb
endif

#---------------
# Static options
#---------------

# Locals
VERSION=$(shell cmd /c type ..\\VERSION)
CXXOPT += -DVERSION="\"$(VERSION)\""
ifeq ($(wildcard ../.svn),../.svn)
    REVISION=$(shell svnversion -n)
else
    REVISION=$(shell cmd /c type ..\\REVISION)
endif
CXXOPT += -DREVISION="\"$(REVISION)\""

COMPONENTS = ITGSend\\ITGSend.exe ITGRecv\\ITGRecv.exe ITGLog\\ITGLog.exe \
	     ITGDec\\ITGDec.exe libITG\\libITG.dll ITGManager\\ITGManager.exe
CLEAN_TARGETS = $(addprefix _,$(notdir $(basename $(subst \\,/,$(COMPONENTS)))))
OSFLAGS = -DWIN32
WARNINGS = -Wall -Wno-deprecated -Wno-strict-aliasing

# Globals
export DEVDIR 
export BUILD_ENV = WIN32_MINGW
export RM = cmd /c del 2>NUL
export CP = cmd /c copy
export MV = cmd /c move
export ECHON = echo | set /p d=
export SUFFIX = .exe
export SOSUFFIX = .dll

export CXX = g++
export RANLIB = ranlib
export AR = ar 
export CXXFLAGS = -O2 $(CXXOPT) $(OSFLAGS) 
export LDFLAGS = $(DEVDIR)/Lib/libws2_32.a 

export BASEDIR = $(shell cmd /c cd)
export BIN = "$(subst src,bin,$(BASEDIR))"
export COMMON = "$(BASEDIR)\common"
export EXEC_DIR = $(BASEDIR)\..\bin

export THOBJS = common/thread.o
export OBJS = common/ITG.o common/timestamp.o common/serial.o common/pipes.o

#-----------
# Main rules
#-----------

.PHONY: all
all: head $(COMPONENTS)
	@ cmd /c echo:
	@ echo ----------------------------------------------------------
	@ echo D-ITG executables created in $(BIN)

head:
	@ cmd /c echo:
	@ echo ---------------------
	@ echo Current configuration
	@ echo ---------------------
	@ $(ECHON)Enabled features*: 
ifdef ipv4only
	@ $(ECHON)ipv4only 
endif
ifdef multiport
	@ $(ECHON)multiport 
endif
ifdef debug
	@ $(ECHON)debug=$(debug) 
endif
	@ cmd /c echo:
	@ echo Version: $(VERSION)
	@ echo Revision: $(REVISION)
	@ echo Compiler: $(CXX)
	@ echo OS: $(OS)
	@ echo OS Flags: $(OSFLAGS)
	@ echo ------------------------------------------------
	@ echo * Run "make help" for available options and tips
	@ cmd /c echo:
	@ echo ---------------------
	@ echo Building common files
	@ echo ---------------------

help:
	@ cmd /c echo:
	@ echo ----------------
	@ echo Building options
	@ echo ----------------
	@ echo    debug=N        : enable debug mode using verbosity level N
	@ echo    ipv4only=on    : disable support for IPv6 (only required on Windows XP to support IPv4)
	@ echo    multiport=on   : enable support for receiving multiple flows on the same port number
	@ echo -------------
	@ echo Building tips
	@ echo -------------
	@ echo  - To build D-ITG with options use the following syntax:
	@ echo     make clean all [options]

#---------------
# Building rules
#---------------
$(COMPONENTS): $(THOBJS) $(OBJS)
	@ cmd /c echo:
	@ echo -----------------------
	@ echo Building $(notdir $@)
	@ echo -----------------------
	@ make -C $(dir $@) --no-print-directory

#-------------
# Common rules
#-------------
include common.mk

	
#---------------
# Cleaning rules
#---------------
clean: clean_head clean_common $(CLEAN_TARGETS)

clean_head:
	@ cmd /c echo:
	@ echo ----------------
	@ echo Cleaning sources
	@ echo ----------------

clean_common:
	@ $(ECHON)Cleaning common files...
	@- $(RM) $(COMMON)\\*.o
	@ echo done

$(CLEAN_TARGETS):
	@ $(ECHON)Cleaning $(subst _,,$@)...
	@ make -C $(subst _,,$@) --no-print-directory clean
	@ echo done

# EOF
