# CS111 section3 Solution Makefile Hooks
PROGS = thread-cycle # thread-cycle-preemption
CXX = /usr/bin/g++

CXX_WARNINGS = -Wall -pedantic -Wno-vla-extension
CXX_DEPS = -MMD -MF $(@:.o=.d)
CXX_DEFINES =
CXX_INCLUDES =

CXXFLAGS = -g $(CXX_WARNINGS) -O0 -std=c++17 $(CXX_DEPS) $(CXX_DEFINES) $(CXX_INCLUDES)
LDFLAGS =

SOURCES = $(PROGS:=.cc)
OBJECTS = $(SOURCES:.cc=.o)

all:: $(PROGS)

$(PROGS): %:%.o #timer.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

clean::
	rm -f $(PROGS) *.o *.s *~ .*~ *.d

spartan:: clean
	\rm -fr *~

.PHONY: all clean spartan

