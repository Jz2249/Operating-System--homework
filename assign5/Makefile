TARGETS = test
EXTRAS = 

PATH_TO_FILE = function.cc
ifneq ("$(wildcard $(PATH_TO_FILE))","")
    EXTRAS += function interrupt interrupt2 static #coroutine
endif
#ARCH = -m32
#CXXBASE = clang++
CXXBASE = g++
CXX = $(CXXBASE) $(ARCH) -std=c++17
CC = $(CXX)
CXXFLAGS = -ggdb -O -Wall -Werror

OBJS = stack.o sync.o test.o thread.o timer.o
HEADERS = stack.hh thread.hh timer.hh sync.hh


all:: $(TARGETS) $(EXTRAS)

$(OBJS): $(HEADERS)

test: $(OBJS) $(LIB)
	$(CXX) -o $@ $(OBJS)
	
coroutine: coroutine.o stack.o
	$(CXX) -o $@ coroutine.o stack.o
	
function: function.cc
	$(CXX) $(CXXFLAGS) function.cc -o function
	
interrupt: interrupt.o timer.o
	$(CXX) -o $@ interrupt.o timer.o 
	
interrupt2: interrupt2.o timer.o
	$(CXX) -o $@ interrupt2.o timer.o
	
static: static.cc
	$(CXX) $(CXXFLAGS) -o $@ static.cc

clean::
	rm -f $(TARGETS) $(EXTRAS) *.o *.s *~ .*~

.SUFFIXES: .cc

.PHONY: all clean

