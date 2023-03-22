# CS111 section3 Solution Makefile Hooks
PROGS = timeout open-file-table # publish subprocesstest 
CXX = /usr/bin/clang++-10

CXX_WARNINGS = -Wall -pedantic -Wno-vla-extension
CXX_DEPS = -MMD -MF $(@:.o=.d)
CXX_DEFINES =
CXX_INCLUDES =

CXXFLAGS = -g $(CXX_WARNINGS) -O0 -std=c++20 $(CXX_DEPS) $(CXX_DEFINES) $(CXX_INCLUDES)
LDFLAGS =

PROGS_SRC = $(patsubst %,%.cc,$(PROGS))
PROGS_OBJ = $(patsubst %.cc,%.o,$(patsubst %.S,%.o,$(PROGS_SRC)))
PROGS_DEP = $(patsubst %.o,%.d,$(PROGS_OBJ))

LIB_SRC = # subprocess.cc
LIB_OBJ = $(patsubst %.cc,%.o,$(patsubst %.S,%.o,$(LIB_SRC)))
LIB_DEP = $(patsubst %.o,%.d,$(LIB_OBJ))
LIB = liblab3.a

all:: $(PROGS)

$(PROGS): %:%.o $(LIB)
	$(CXX) $^ $(LDFLAGS) -o $@ 

$(LIB): $(LIB_OBJ)
	rm -f $@
	ar r $@ $^
	ranlib $@

clean::
	rm -f $(PROGS) $(PROGS_OBJ) $(PROGS_DEP)
	rm -f $(LIB) $(LIB_OBJ) $(LIB_DEP)

spartan:: clean
	\rm -fr *~

.PHONY: all clean spartan

-include $(PROGS_DEP)

