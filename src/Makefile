# D-ITG Main Makefile
#
# It automatically includes the right mk files depending 
# on the host and the T

# Host OS detection
ifndef OS
	OS := $(shell uname)
endif

# Makefile selection
ifeq ($(OS),Linux)
	ifeq ($(T),win)
		MKFILE := linux-mingw.mk
	else
		MKFILE := unix.mk
	endif
endif
ifeq ($(OS),FreeBSD)
	ifndef T
		MKFILE := unix.mk
	endif
endif
ifeq ($(OS),Darwin)
	ifndef T
		MKFILE := unix.mk
	endif
endif
ifeq ($(OS),Windows_NT)
	ifndef T
		MKFILE := win-mingw.mk
	endif
endif

# Include the right mk file
ifdef MKFILE
	include $(MKFILE)
else
error:
ifdef T
	@ echo "Cross-compiling for $(T) target is not currently supported"
else
	@ echo "$(OS) currently not supported"
endif
endif

# EOF
