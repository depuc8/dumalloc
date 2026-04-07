# Makefile — dumalloc build system
#
# Targets:
#   all      — build the test driver (default)
#   lib      — build a static library (libdumalloc.a)
#   clean    — remove all build artefacts

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g
AR      := ar
ARFLAGS := rcs

TARGET  := dumalloc
LIB     := libdumalloc.a
SRCS    := heap.c main.c
OBJS    := $(SRCS:.c=.o)
LIB_OBJ := heap.o

.PHONY: all lib clean

## Build the test driver
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

## Build a static library containing just the allocator
lib: $(LIB)

$(LIB): $(LIB_OBJ)
	$(AR) $(ARFLAGS) $@ $^

## Pattern rule: compile a .c file into a .o file
%.o: %.c heap.h
	$(CC) $(CFLAGS) -c -o $@ $<

## Remove all build artefacts
clean:
	rm -f $(OBJS) $(TARGET) $(LIB)
