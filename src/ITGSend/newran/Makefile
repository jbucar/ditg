# D-ITG newran Makefile wrapper 

all: libnewran

#-----------------
include nr_gnu.mak
#-----------------

libnewran: $(newran_lobj)
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ AR ]  $@.a ^<- $(newran_lobj)
else
	@ printf "[ AR ]\t$@.a <- $(newran_lobj)\n"
endif
	@ $(AR) -cr $@.a $(newran_lobj)
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ RL ]  $@.a
else
	@ printf "[ RL ]\t$@.a\n"
endif
	@ $(RANLIB) $@.a
	
clean:
	@- $(RM) *.o *.a


#---------------------
include ../../common.mk
#---------------------
