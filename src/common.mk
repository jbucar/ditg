# D-ITG common Makefile rules

%.o: %.cpp %.h
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ CXX ]  $@ ^<- $<
else
	@ printf "[ CXX ]\t$@ <- $<\n"
endif
	@ $(CXX) -c $(CXXFLAGS) $(INCS) -o $@ $<

%.o: %.cpp
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ CXX ]  $@ ^<- $<
else
	@ printf "[ CXX ]\t$@ <- $<\n"
endif
	@ $(CXX) -c $(CXXFLAGS) $(INCS) -o $@ $<

