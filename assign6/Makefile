TARGETS = test vm_demo

CXXBASE = c++
CXX = $(CXXBASE) -std=c++17
CXXFLAGS = -ggdb -Wall -Werror

CPPFLAGS = $$(pkg-config --cflags libcrypto)
LIBS = $$(pkg-config --libs libcrypto)

OBJS = mcryptfile.o cryptfile.o crypto.o vm.o test.o
HEADERS = cryptfile.hh crypto.hh mcryptfile.hh util.hh vm.hh

TEST_HARNESS_PATH = test_harness.cc
ifneq ("$(wildcard $(TEST_HARNESS_PATH))","")
	TARGETS = test_harness
	EXTRAS =
	OBJS = vm.o test_harness.o virtualmemoryregion.o physicalmemory.o diskregion.o
	HEADERS = util.hh vm.hh virtualmemoryregion.hh physicalmemory.hh diskregion.hh
	CPPFLAGS =
	LIBS =
endif


all: $(TARGETS) $(SOLN_PROG)

$(OBJS): $(HEADERS)

test: $(OBJS) $(LIB)
	$(CXX) -o $@ $(OBJS) $(LIBS)

test_harness: $(OBJS) $(LIB)
	$(CXX) -o $@ $(OBJS) $(LIBS)

vm_demo: vm_demo.o vm.o
	$(CXX) -o $@ vm_demo.o vm.o


clean:
	rm -f $(TARGETS) $(LIB) *.o *~ .*~

.PHONY: all clean
