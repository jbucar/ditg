OBJS = ../common/ITG.o ../common/timestamp.o ../$(THOBJS) channel.o ITGLog.o

all:   ITGLog$(SUFFIX)

ITGLog$(SUFFIX): ../common/ITG.h $(OBJS)
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ LD ]  $@ ^<- $(OBJS)
else
	@ printf "[ LD ]\t$@ <- $(OBJS)\n"
endif
	@ $(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	@ $(CP) $@ $(BIN)

clean:
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@- $(RM) *.o ITGLog$(SUFFIX) $(BIN)\ITGLog$(SUFFIX)
else
	@ $(RM) *.o ITGLog$(SUFFIX) $(BIN)/ITGLog$(SUFFIX)
endif

#---------------
include ../common.mk
#---------------
