CXX = c++ -std=c++17
CXXFLAGS = -ggdb -O -Wall -Werror

PROGS = test
TARGETS = $(PROGS)

OBJS = virtual_memory.o test.o diskregion.o vm.o

all:: $(TARGETS)

test: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJS): $(HEADERS)

clean::
	rm -f $(TARGETS) $(OBJS) *~ .*~

.PHONY: all clean

