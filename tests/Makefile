CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g

INCDIR  := ../include
SRCDIR  := ../src
BUILDDIR := build
BINDIR  := bin

TEST_BIN := $(BINDIR)/test_alloc
TEST_SRC := test_alloc.c $(SRCDIR)/heap.c
TEST_OBJ := $(BUILDDIR)/test_alloc.o $(BUILDDIR)/heap.o

.PHONY: all run clean

all: $(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/test_alloc.o: test_alloc.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

$(BUILDDIR)/heap.o: $(SRCDIR)/heap.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

run: $(TEST_BIN)
	@./$(TEST_BIN)

clean:
	rm -rf $(BUILDDIR) $(BINDIR)
