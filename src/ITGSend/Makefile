NRDIR = newran
OBJS := ../$(THOBJS) $(addprefix ../,$(OBJS)) traffic.o ITGSend.o

all: ITGSend$(SUFFIX)

ITGSend$(SUFFIX): ../common/ITG.h libnewran.a $(OBJS)
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ LD ]  $@ ^<- $(OBJS) libnewran.a
else
	@ printf "[ LD ]\t$@ <- $(OBJS) libnewran.a\n"
endif
	@ $(CXX) $(CXXFLAGS) -I$(NRDIR) -L$(NRDIR) $(OBJS) -lnewran -o $@ $(LDFLAGS)
	@ $(CP) $@ $(BIN)

libnewran.a: newran
	@ $(MAKE) -C $(NRDIR) -ef Makefile libnewran

clean:
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@- $(RM) *.o ITGSend$(SUFFIX) $(BIN)\ITGSend$(SUFFIX)
else
	@ $(RM) *.o ITGSend$(SUFFIX) $(BIN)/ITGSend$(SUFFIX)
endif
	@ $(MAKE) -C $(NRDIR) clean

#------------------
include ../common.mk
#------------------
