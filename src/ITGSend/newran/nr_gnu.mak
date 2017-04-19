CXX = g++
CXXFLAGS = -O2 -Wall

DIFF = ./sdiff
PRE = ./
MAJOR = 1
MINOR = 0

%.o:           	%.cpp
		$(CXX) $(CXXFLAGS) -c $*.cpp

everything:    	tryrand tryurng geturng nr_ex test_lg 

newran_lobj = newran1.o newran2.o myexcept.o simpstr.o extreal.o

libnewran.a:   	$(newran_lobj)
		$(AR) -cr $@ $(newran_lobj)
		$(RANLIB) $@

tryrand_obj = tryrand.o format.o utility.o tryrand1.o tryrand2.o tryrand3.o tryrand4.o tryrand5.o tryrand6.o str.o test_out.o

tryrand:       	$(tryrand_obj) libnewran.a
		$(CXX) -o $@ $(tryrand_obj) -L. -lnewran -lm

tryurng_obj = tryurng.o format.o tryurng1.o utility.o test_out.o str.o

tryurng:       	$(tryurng_obj) libnewran.a
		$(CXX) -o $@ $(tryurng_obj) -L. -lnewran -lm

geturng_obj = geturng.o

geturng:       	$(geturng_obj) libnewran.a
		$(CXX) -o $@ $(geturng_obj) -L. -lnewran -lm

nr_ex_obj = nr_ex.o

nr_ex:         	$(nr_ex_obj) libnewran.a
		$(CXX) -o $@ $(nr_ex_obj) -L. -lnewran -lm

test_lg_obj = test_lg.o format.o str.o

test_lg:       	$(test_lg_obj) libnewran.a
		$(CXX) -o $@ $(test_lg_obj) -L. -lnewran -lm

newran1.o:     	newran1.cpp include.h newran.h myexcept.h extreal.h simpstr.h

newran2.o:     	newran2.cpp include.h newran.h myexcept.h extreal.h simpstr.h

myexcept.o:    	myexcept.cpp include.h myexcept.h

simpstr.o:     	simpstr.cpp include.h myexcept.h simpstr.h

extreal.o:     	extreal.cpp include.h extreal.h

tryrand.o:     	tryrand.cpp include.h newran.h format.h utility.h tryrand.h myexcept.h extreal.h simpstr.h str.h

format.o:      	format.cpp include.h str.h format.h myexcept.h

utility.o:     	utility.cpp include.h myexcept.h utility.h format.h str.h

tryrand1.o:    	tryrand1.cpp include.h newran.h format.h utility.h tryrand.h myexcept.h extreal.h simpstr.h str.h

tryrand2.o:    	tryrand2.cpp include.h newran.h format.h utility.h tryrand.h myexcept.h extreal.h simpstr.h str.h

tryrand3.o:    	tryrand3.cpp include.h newran.h format.h tryrand.h utility.h test_out.h myexcept.h extreal.h simpstr.h str.h

tryrand4.o:    	tryrand4.cpp include.h newran.h str.h test_out.h format.h tryrand.h utility.h array.h myexcept.h extreal.h simpstr.h

tryrand5.o:    	tryrand5.cpp include.h newran.h format.h tryrand.h utility.h myexcept.h extreal.h simpstr.h str.h

tryrand6.o:    	tryrand6.cpp include.h newran.h format.h tryrand.h utility.h test_out.h myexcept.h extreal.h simpstr.h str.h

str.o:         	str.cpp include.h str.h myexcept.h

test_out.o:    	test_out.cpp include.h str.h utility.h test_out.h format.h myexcept.h

tryurng.o:     	tryurng.cpp include.h newran.h format.h tryurng.h utility.h test_out.h myexcept.h extreal.h simpstr.h str.h

tryurng1.o:    	tryurng1.cpp include.h newran.h format.h tryurng.h utility.h test_out.h array.h myexcept.h extreal.h simpstr.h str.h

geturng.o:     	geturng.cpp include.h newran.h myexcept.h extreal.h simpstr.h

nr_ex.o:       	nr_ex.cpp newran.h include.h myexcept.h extreal.h simpstr.h

test_lg.o:     	test_lg.cpp include.h format.h newran.h str.h myexcept.h extreal.h simpstr.h

tryrand.txx:   	tryrand
		$(PRE)tryrand > tryrand.txx
		$(DIFF) tryrand.txt tryrand.txx

tryurng.txx:   	tryurng
		$(PRE)tryurng > tryurng.txx
		$(DIFF) tryurng.txt tryurng.txx

geturng.txx:   	geturng
		$(PRE)geturng > geturng.txx
		$(DIFF) geturng.txt geturng.txx

nr_ex.txx:     	nr_ex
		$(PRE)nr_ex > nr_ex.txx
		$(DIFF) nr_ex.txt nr_ex.txx

test_lg.txx:   	test_lg
		$(PRE)test_lg > test_lg.txx
		$(DIFF) test_lg.txt test_lg.txx

