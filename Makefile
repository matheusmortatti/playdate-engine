# Playdate Engine Memory Management Tests
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -DDEBUG -O2
INCLUDES = -I.
SRCDIR = src/core
TESTDIR = tests/core

# Source files
SOURCES = $(SRCDIR)/memory_pool.c $(SRCDIR)/memory_debug.c
TEST_SOURCES = $(TESTDIR)/test_memory_pool.c $(TESTDIR)/test_memory_perf.c $(TESTDIR)/test_memory_debug.c $(TESTDIR)/test_runner.c

# Object files
OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Executables
TEST_RUNNER = test_runner

.PHONY: all clean test test-verbose

all: $(TEST_RUNNER)

$(TEST_RUNNER): $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $@

# Individual object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile and run tests
test: 
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) $(TEST_SOURCES) -o $(TEST_RUNNER)
	./$(TEST_RUNNER)

# Verbose test output
test-verbose: CFLAGS += -DVERBOSE_TESTS
test-verbose: test

# Individual test builds
test-pool:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(SRCDIR)/memory_pool.c $(SRCDIR)/memory_debug.c $(TESTDIR)/test_memory_pool.c -o test_pool
	./test_pool

test-perf:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(SRCDIR)/memory_pool.c $(SRCDIR)/memory_debug.c $(TESTDIR)/test_memory_perf.c -o test_perf
	./test_perf

test-debug:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(SRCDIR)/memory_pool.c $(SRCDIR)/memory_debug.c $(TESTDIR)/test_memory_debug.c -o test_debug
	./test_debug

# Clean up
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TEST_RUNNER) test_pool test_perf test_debug