#include <stdio.h>

// External test function declarations
extern int run_scene_tests(void);
extern int run_scene_performance_tests(void);

int main(void) {
    printf("=== Playdate Engine Scene Management Test Suite ===\n\n");
    
    int total_failures = 0;
    
    // Run unit tests
    printf("PHASE 4.1: Scene Unit Tests\n");
    printf("============================\n");
    total_failures += run_scene_tests();
    
    // Run performance tests
    printf("PHASE 4.2: Scene Performance Tests\n");
    printf("===================================\n");
    total_failures += run_scene_performance_tests();
    
    // Summary
    printf("=== Test Suite Summary ===\n");
    if (total_failures == 0) {
        printf("🎉 ALL SCENE TESTS PASSED! 🎉\n");
        printf("Scene management system is ready for integration.\n");
    } else {
        printf("❌ %d test(s) failed\n", total_failures);
        printf("Please fix issues before proceeding to Phase 5.\n");
    }
    printf("===========================\n\n");
    
    return total_failures;
}