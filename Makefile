CXX = g++
CXXFLAGS = -g -Wall -std=c++20 -Iinclude -Igensrc

SRCS = main.cpp operand.cpp opcode.cpp instruction.cpp \
	lexer.cpp parser.cpp location.cpp
OBJS = $(SRCS:%.cpp=build/%.o)

GENSRCS = leximpl.cpp

# You only need this if you want to regenerate the
# generated lexer source code.
REGEX_TOOLKIT = $(HOME)/git/RegexToolkit/RegexToolkit/regexToolkit.jar

build/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) -c src/$*.cpp -o build/$*.o

build/hlsim : $(OBJS)
	$(CXX) -o $@ $+

gensrc/leximpl.cpp : src/lexspec.txt
	java -jar $(REGEX_TOOLKIT) lexgen $+ $@

depend : $(GENSRCS:%=gensrc/%)
	$(CXX) $(CXXFLAGS) -M $(SRCS:%=src/%) \
		| perl -ne 's,^(\S),build/$$1,g;print' \
		> build/depend.mak

build/depend.mak :
	touch $@

clean :
	rm -f build/hlsim build/*.o build/depend.mak

include build/depend.mak
