CFLAGS = $(subst -O2,-O0,$(CXXFLAGS))
OBJS = ITGDecod.o ../common/ITG.o

all: ITGDec$(SUFFIX) 

ITGDec$(SUFFIX): ITGDecod.cpp $(OBJS)
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ LD ]  $@ ^<- $(OBJS)
else
	@ printf "[ LD ]\t$@ <- $(OBJS)\n"
endif
	@ $(CXX) $(CFLAGS) $(OBJS) -lm -o $@ $(LDFLAGS)
	@ $(CP) $@ $(BIN)

clean:
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@- $(RM) *.o ITGDec$(SUFFIX) $(BIN)\ITGDec$(SUFFIX)
else
	@ $(RM) *.o ITGDec$(SUFFIX) $(BIN)/ITGDec$(SUFFIX)
endif

#---------------
include ../common.mk
#---------------
