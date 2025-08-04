# Playdate Engine Build System
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -DDEBUG -O2 -lm
INCLUDES = -I.

# Directory structure
CORE_SRCDIR = src/core
COMPONENTS_SRCDIR = src/components
CORE_TESTDIR = tests/core
COMPONENTS_TESTDIR = tests/components

# Phase 1: Memory management sources
MEMORY_SOURCES = $(CORE_SRCDIR)/memory_pool.c $(CORE_SRCDIR)/memory_debug.c
MEMORY_TEST_SOURCES = $(CORE_TESTDIR)/test_memory_pool.c $(CORE_TESTDIR)/test_memory_perf.c $(CORE_TESTDIR)/test_memory_debug.c $(CORE_TESTDIR)/test_runner.c

# Phase 2: Component system sources
COMPONENT_SOURCES = $(CORE_SRCDIR)/component.c $(CORE_SRCDIR)/component_registry.c $(COMPONENTS_SRCDIR)/transform_component.c $(COMPONENTS_SRCDIR)/component_factory.c
COMPONENT_TEST_SOURCES = $(CORE_TESTDIR)/test_component.c $(CORE_TESTDIR)/test_component_registry.c $(CORE_TESTDIR)/test_component_perf.c $(COMPONENTS_TESTDIR)/test_transform.c $(COMPONENTS_TESTDIR)/test_component_factory.c $(CORE_TESTDIR)/test_component_runner.c

# Combined sources
ALL_SOURCES = $(MEMORY_SOURCES) $(COMPONENT_SOURCES)
ALL_TEST_SOURCES = $(MEMORY_TEST_SOURCES) $(COMPONENT_TEST_SOURCES)

# Object files
MEMORY_OBJECTS = $(MEMORY_SOURCES:.c=.o)
COMPONENT_OBJECTS = $(COMPONENT_SOURCES:.c=.o)
ALL_OBJECTS = $(ALL_SOURCES:.c=.o)

# Executables
MEMORY_TEST_RUNNER = test_memory_system
COMPONENT_TEST_RUNNER = test_component_system

.PHONY: all clean test test-verbose test-memory test-components test-all

# Default target - run all tests
all: test-all

# Individual object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Memory system tests (Phase 1)
test-memory: 
	$(CC) $(CFLAGS) $(INCLUDES) $(MEMORY_SOURCES) $(MEMORY_TEST_SOURCES) -o $(MEMORY_TEST_RUNNER)
	./$(MEMORY_TEST_RUNNER)

# Component system tests (Phase 2)
test-components:
	$(CC) $(CFLAGS) $(INCLUDES) $(ALL_SOURCES) $(COMPONENT_TEST_SOURCES) -o $(COMPONENT_TEST_RUNNER)
	./$(COMPONENT_TEST_RUNNER)

# Run all tests
test-all: test-memory test-components

# Legacy test target for backward compatibility
test: test-memory

# Verbose test output
test-verbose: CFLAGS += -DVERBOSE_TESTS
test-verbose: test-all

# Individual component test builds
test-component-core:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(ALL_SOURCES) $(CORE_TESTDIR)/test_component.c -o test_component_core
	./test_component_core

test-component-registry:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(ALL_SOURCES) $(CORE_TESTDIR)/test_component_registry.c -o test_component_registry
	./test_component_registry

test-component-perf:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(ALL_SOURCES) $(CORE_TESTDIR)/test_component_perf.c -o test_component_perf
	./test_component_perf

test-transform:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(ALL_SOURCES) $(COMPONENTS_TESTDIR)/test_transform.c -o test_transform
	./test_transform

test-factory:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(ALL_SOURCES) $(COMPONENTS_TESTDIR)/test_component_factory.c -o test_factory
	./test_factory

# Individual memory test builds (legacy)
test-pool:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(MEMORY_SOURCES) $(CORE_TESTDIR)/test_memory_pool.c -o test_pool
	./test_pool

test-perf:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(MEMORY_SOURCES) $(CORE_TESTDIR)/test_memory_perf.c -o test_perf
	./test_perf

test-debug:
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_STANDALONE $(MEMORY_SOURCES) $(CORE_TESTDIR)/test_memory_debug.c -o test_debug
	./test_debug

# Clean up
clean:
	rm -f $(ALL_OBJECTS) $(MEMORY_TEST_RUNNER) $(COMPONENT_TEST_RUNNER) test_*

# Development shortcuts
.PHONY: quick-test
quick-test:
	@echo "Running quick component system validation..."
	$(CC) $(CFLAGS) $(INCLUDES) $(ALL_SOURCES) $(COMPONENT_TEST_SOURCES) -o $(COMPONENT_TEST_RUNNER) && ./$(COMPONENT_TEST_RUNNER)