#include "../../src/core/memory_pool.h"
#include "../../src/core/memory_debug.h"
#include <stdio.h>

// External test function declarations
extern int run_memory_pool_tests(void);
extern int run_memory_performance_tests(void);
extern int run_memory_debug_tests(void);

int main(void) {
    printf("=== Playdate Engine Memory Management Test Suite ===\n\n");
    
    int total_failures = 0;
    
    // Run unit tests
    printf("PHASE 1: Unit Tests\n");
    printf("===================\n");
    total_failures += run_memory_pool_tests();
    
    // Run performance tests
    printf("PHASE 2: Performance Tests\n");
    printf("===========================\n");
    total_failures += run_memory_performance_tests();
    
    // Run debug tests
    printf("PHASE 3: Debug System Tests\n");
    printf("============================\n");
    total_failures += run_memory_debug_tests();
    
    // Summary
    printf("=== Test Suite Summary ===\n");
    if (total_failures == 0) {
        printf("🎉 ALL TESTS PASSED! 🎉\n");
        printf("Memory management system is ready for Phase 2.\n");
    } else {
        printf("❌ %d test(s) failed\n", total_failures);
        printf("Please fix issues before proceeding to Phase 2.\n");
    }
    printf("===========================\n\n");
    
    return total_failures;
}