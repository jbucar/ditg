all: ITGManager$(SUFFIX)

OBJS = ../common/thread.o ITGManager.o

ITGManager$(SUFFIX): ../libITG/ITGapi.h $(OBJS)
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ LD ]  $@ ^<- $(OBJS)
else
	@ printf "[ LD ]\t$@ <- $(OBJS)\n"
endif
	@ $(CXX) $(CXXFLAGS) $(OBJS) -L$(BIN) -lITG -o $@ $(LDFLAGS) 
	@ $(CP) $@ $(BIN)

clean:
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@- $(RM) *.o ITGManager$(SUFFIX) $(BIN)\ITGManager$(SUFFIX)
else
	@ $(RM) *.o ITGManager$(SUFFIX) $(BIN)/ITGManager$(SUFFIX)
endif

#---------------
include ../common.mk
#---------------
