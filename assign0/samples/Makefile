PROGS = buggy moviecredits movietest
CXX = g++ -std=c++17
CXXFLAGS = -ggdb -O -Wall -Werror

CC = /usr/bin/clang-10
DEPS = -MMD -MF $(@:.o=.d)
WARNINGS = -fstack-protector -Wall -W -Wcast-qual -Wwrite-strings -Wextra -Wno-unused -Wno-unused-parameter

CFLAGS = -g -fno-limit-debug-info $(WARNINGS) $(DEPS) -std=gnu99

all:: $(PROGS)

buggy: %:%.c
	$(CC) $(CFLAGS) $^ -o $@

movietest: %:%.cc movie.cc
	$(CXX) $(CXXFLAGS) $^ -o $@

clean::
	rm -f $(PROGS) *.o *~ .*~

.PHONY: clean all

