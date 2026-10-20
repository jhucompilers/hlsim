CXX = g++
CXXFLAGS = -g -Wall -std=c++20 -Iinclude

SRCS = main.cpp operand.cpp
OBJS = $(SRCS:%.cpp=build/%.o)

build/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) -c src/$*.cpp -o build/$*.o

build/hlsim : $(OBJS)
	$(CXX) -o $@ $+

depend :
	$(CXX) $(CXXFLAGS) -M $(SRCS:%=src/%) \
		| perl -ne 's,^(\S),build/$$1,g;print' \
		> build/depend.mak

build/depend.mak :
	touch $@

clean :
	rm -f build/hlsim build/*.o build/depend.mak

include build/depend.mak
