ifdef UNIX
	CXXFLAGS += -fPIC
endif

OBJS = ITGapi.o ../$(THOBJS)

all:   libITG$(SOSUFFIX)

libITG$(SOSUFFIX): ITGapi.h ITGapi.o
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ LD ]  $@ ^<- $(OBJS)
else
	@ printf "[ LD ]\t$@ <- $(OBJS)\n"
endif
	@ $(CXX) $(CXXFLAGS) -shared -o $@ $(OBJS) $(LDFLAGS)
	@ $(CP) $@ $(BIN)

clean:
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@- $(RM) *.o libITG$(SOSUFFIX) $(BIN)\libITG$(SOSUFFIX)
else
	@ $(RM) *.o libITG$(SOSUFFIX) $(BIN)/libITG$(SOSUFFIX)
endif

#---------------
include ../common.mk
#---------------
