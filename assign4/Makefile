CXX = g++ -std=c++17
CXXFLAGS = -ggdb -O -Wall -Werror

PROGS = caltrain_test party_test
TARGETS = $(PROGS) car-simulation car-simulation-two-locks

# Compile destruct.cc only if present (StackOverflow post 5553352)
PATH_TO_FILE = destruct.cc
ifneq ("$(wildcard $(PATH_TO_FILE))","")
    TARGETS += destruct
endif

OBJS = caltrain.o caltrain_test.o party.o party_test.o bridge.o bridge-two-locks.o car-simulation.o car-simulation-two-locks.o

all:: $(TARGETS)

test: $(TARGETS)
	./run_tests

caltrain_test: caltrain_test.o caltrain.o
	$(CXX) $(CXXFLAGS) $^ -pthread -o $@

party_test: party_test.o party.o
	$(CXX) $(CXXFLAGS) $^ -pthread -o $@

destruct: destruct.cc
	$(CXX) $(CXXFLAGS) destruct.cc -o destruct

car-simulation: car-simulation.o bridge.o
	$(CXX) $(CXXFLAGS) $^ -pthread -L/usr/class/cs110/local/lib/ -lthreads -lrand -o $@

car-simulation-two-locks: car-simulation-two-locks.o bridge-two-locks.o
	$(CXX) $(CXXFLAGS) $^ -pthread -L/usr/class/cs110/local/lib/ -lthreads -lrand -o $@

$(OBJS): $(HEADERS)

clean::
	rm -f $(TARGETS) $(OBJS) *~ .*~

.PHONY: all clean

