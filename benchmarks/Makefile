CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -O2

INCDIR  := ../include
SRCDIR  := ../src
BUILDDIR := build
BINDIR  := bin

BENCH_BIN := $(BINDIR)/bench
BENCH_OBJ := $(BUILDDIR)/bench.o $(BUILDDIR)/heap.o

.PHONY: all run clean

all: $(BENCH_BIN)

$(BENCH_BIN): $(BENCH_OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/bench.o: bench.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

$(BUILDDIR)/heap.o: $(SRCDIR)/heap.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

run: $(BENCH_BIN)
	@./$(BENCH_BIN)

clean:
	rm -rf $(BUILDDIR) $(BINDIR)
