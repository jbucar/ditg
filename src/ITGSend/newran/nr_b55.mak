BORLANDPATH = "C:\program files\Borland\cbuilder5"

TASM = TASM32
TLIB = tlib
TLINK = ilink32
LIBPATH = $(BORLANDPATH)\LIB
INCLUDEPATH = $(BORLANDPATH)\INCLUDE

DIFF = sdiff
PRE =

CC = bcc32 -W- -v- -H- -3 -N -Og -Oi -Ov -f -I$(INCLUDEPATH)

.cpp.obj:
   $(CC) -c {$< }

everything:    	tryrand.exe tryurng.exe geturng.exe nr_ex.exe test_lg.exe 

newran_lobj = newran1.obj newran2.obj myexcept.obj simpstr.obj extreal.obj

newran.lib:    	$(newran_lobj)
   $(TLIB) $@ /P32 /u $(newran_lobj)

tryrand_obj = tryrand.obj format.obj utility.obj tryrand1.obj tryrand2.obj tryrand3.obj tryrand4.obj tryrand5.obj tryrand6.obj str.obj test_out.obj

tryrand.exe:   	$(tryrand_obj) newran.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(tryrand_obj),$@,, newran.lib import32.lib cw32.lib
|

tryurng_obj = tryurng.obj format.obj tryurng1.obj utility.obj test_out.obj str.obj

tryurng.exe:   	$(tryurng_obj) newran.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(tryurng_obj),$@,, newran.lib import32.lib cw32.lib
|

geturng_obj = geturng.obj

geturng.exe:   	$(geturng_obj) newran.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(geturng_obj),$@,, newran.lib import32.lib cw32.lib
|

nr_ex_obj = nr_ex.obj

nr_ex.exe:     	$(nr_ex_obj) newran.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(nr_ex_obj),$@,, newran.lib import32.lib cw32.lib
|

test_lg_obj = test_lg.obj format.obj str.obj

test_lg.exe:   	$(test_lg_obj) newran.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(test_lg_obj),$@,, newran.lib import32.lib cw32.lib
|

newran1.obj:   	newran1.cpp include.h newran.h myexcept.h extreal.h simpstr.h

newran2.obj:   	newran2.cpp include.h newran.h myexcept.h extreal.h simpstr.h

myexcept.obj:  	myexcept.cpp include.h myexcept.h

simpstr.obj:   	simpstr.cpp include.h myexcept.h simpstr.h

extreal.obj:   	extreal.cpp include.h extreal.h

tryrand.obj:   	tryrand.cpp include.h newran.h format.h utility.h tryrand.h myexcept.h extreal.h simpstr.h str.h

format.obj:    	format.cpp include.h str.h format.h myexcept.h

utility.obj:   	utility.cpp include.h myexcept.h utility.h format.h str.h

tryrand1.obj:  	tryrand1.cpp include.h newran.h format.h utility.h tryrand.h myexcept.h extreal.h simpstr.h str.h

tryrand2.obj:  	tryrand2.cpp include.h newran.h format.h utility.h tryrand.h myexcept.h extreal.h simpstr.h str.h

tryrand3.obj:  	tryrand3.cpp include.h newran.h format.h tryrand.h utility.h test_out.h myexcept.h extreal.h simpstr.h str.h

tryrand4.obj:  	tryrand4.cpp include.h newran.h str.h test_out.h format.h tryrand.h utility.h array.h myexcept.h extreal.h simpstr.h

tryrand5.obj:  	tryrand5.cpp include.h newran.h format.h tryrand.h utility.h myexcept.h extreal.h simpstr.h str.h

tryrand6.obj:  	tryrand6.cpp include.h newran.h format.h tryrand.h utility.h test_out.h myexcept.h extreal.h simpstr.h str.h

str.obj:       	str.cpp include.h str.h myexcept.h

test_out.obj:  	test_out.cpp include.h str.h utility.h test_out.h format.h myexcept.h

tryurng.obj:   	tryurng.cpp include.h newran.h format.h tryurng.h utility.h test_out.h myexcept.h extreal.h simpstr.h str.h

tryurng1.obj:  	tryurng1.cpp include.h newran.h format.h tryurng.h utility.h test_out.h array.h myexcept.h extreal.h simpstr.h str.h

geturng.obj:   	geturng.cpp include.h newran.h myexcept.h extreal.h simpstr.h

nr_ex.obj:     	nr_ex.cpp newran.h include.h myexcept.h extreal.h simpstr.h

test_lg.obj:   	test_lg.cpp include.h format.h newran.h str.h myexcept.h extreal.h simpstr.h

tryrand.txx:   	tryrand.exe
		$(PRE)tryrand > tryrand.txx
		$(DIFF) tryrand.txt tryrand.txx

tryurng.txx:   	tryurng.exe
		$(PRE)tryurng > tryurng.txx
		$(DIFF) tryurng.txt tryurng.txx

geturng.txx:   	geturng.exe
		$(PRE)geturng > geturng.txx
		$(DIFF) geturng.txt geturng.txx

nr_ex.txx:     	nr_ex.exe
		$(PRE)nr_ex > nr_ex.txx
		$(DIFF) nr_ex.txt nr_ex.txx

test_lg.txx:   	test_lg.exe
		$(PRE)test_lg > test_lg.txx
		$(DIFF) test_lg.txt test_lg.txx

