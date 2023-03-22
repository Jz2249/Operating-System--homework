# CS111 Makefile Hooks: news-aggregator

# Compile parallel jobs by default.
MAKEFLAGS := --jobs=$(shell nproc) --output-sync=target

PROGS = news-aggregator
CXX = clang++-10


WARNINGS = -Wall -pedantic
DEPS = -MMD -MF $(@:.o=.d)
DEFINES = -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD
INCLUDES = -I/afs/ir/class/cs110/local/include -I/usr/include/libxml2 \
					-isystem /afs/ir/class/cs110/include/netlib \
					-I/afs/ir/class/cs110/include/myhtml

CXXFLAGS = -g -fno-limit-debug-info $(WARNINGS) -fPIE -O0 -std=c++20 $(DEPS) $(DEFINES) $(INCLUDES)
LDFLAGS = -lm -lxml2 -L/afs/ir/class/cs110/local/lib -lrand -lthreads -lrssnet -pthread -lboost_thread \
          -L/afs/ir/class/cs110/lib/netlib -lcppnetlib-client-connections -lcppnetlib-uri -lcppnetlib-server-parsers \
          -L/afs/ir/class/cs110/lib/myhtml -lmyhtml -lssl -lcrypto -ldl

NA_LIB = libna.a

PROGS_SRC = news-aggregator.cc
PROGS_OBJ = $(patsubst %.cc,%.o,$(patsubst %.S,%.o,$(PROGS_SRC)))
PROGS_DEP = $(patsubst %.o,%.d,$(PROGS_OBJ))

all:: $(PROGS)

$(PROGS): %:%.o $(NA_LIB)
	$(CXX) $^ $(LDFLAGS) -o $@

clean::
	rm -f $(PROGS) $(PROGS_OBJ) $(PROGS_DEP)

spartan: clean
	@\rm -fr *~

.PHONY: all clean spartan

-include $(PROGS_DEP)

