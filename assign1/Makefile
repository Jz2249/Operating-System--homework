# Makefile for CS 111 project to read Unix V6 file system

CC = /usr/bin/clang-10

PROG = diskimageaccess function_tester

LIB_SRC  = diskimg.c inode.c unixfilesystem.c directory.c pathname.c chksumfile.c file.c 
DEPS = -MMD -MF $(@:.o=.d)
WARNINGS = -fstack-protector -Wall -W -Wcast-qual -Wwrite-strings -Wextra -Wno-unused -Wno-unused-parameter

CFLAGS = -g -fno-limit-debug-info $(WARNINGS) $(DEPS) -std=gnu99
LDFLAGS =

LIB_OBJ = $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(LIB_SRC)))
LIB_DEP = $(patsubst %.o,%.d,$(LIB_OBJ))
LIB = v6fslib.a 

PROG_SRC = diskimageaccess.c function_tester.c
PROG_OBJ = $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(PROG_SRC)))
PROG_DEP = $(patsubst %.o,%.d,$(PROG_OBJ))

TMP_PATH := /usr/bin:$(PATH)
export PATH = $(TMP_PATH)

LIBS += -lssl -lcrypto -lm

all:: $(PROG)

diskimageaccess: diskimageaccess.o $(LIB)
	$(CC) $(LDFLAGS) diskimageaccess.o $(LIB) $(LIBS) -o $@

function_tester: function_tester.o $(LIB)
	$(CC) $(LDFLAGS) function_tester.o $(LIB) $(LIBS) -o $@

$(LIB): $(LIB_OBJ)
	rm -f $@
	ar r $@ $^
	ranlib $@

clean::
	rm -f $(PROG) $(PROG_OBJ) $(PROG_DEP)
	rm -f $(LIB) $(LIB_DEP) $(LIB_OBJ)

.PHONY: all clean 

-include $(LIB_DEP) $(PROG_DEP)

