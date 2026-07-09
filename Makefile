CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g
AR      := ar
ARFLAGS := rcs

SRCDIR  := src
INCDIR  := include
BUILDDIR := build
BINDIR  := bin

TARGET  := $(BINDIR)/dumalloc
LIB     := $(BINDIR)/libdumalloc.a

SRCS    := $(SRCDIR)/heap.c $(SRCDIR)/main.c
OBJS    := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))
LIB_OBJ := $(BUILDDIR)/heap.o

.PHONY: all lib test bench clean

all: $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

lib: $(LIB)

$(LIB): $(LIB_OBJ) | $(BINDIR)
	$(AR) $(ARFLAGS) $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

test:
	$(MAKE) -C tests run

bench:
	$(MAKE) -C benchmarks run

clean:
	rm -rf $(BUILDDIR) $(BINDIR)
	$(MAKE) -C tests clean
	$(MAKE) -C benchmarks clean
