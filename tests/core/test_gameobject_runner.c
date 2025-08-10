#include <stdio.h>
#include <stdlib.h>

// Test function declarations
int run_game_object_tests(void);
int run_gameobject_performance_tests(void);

int main(void) {
    printf("=== Phase 3: GameObject & Transform Test Suite ===\n\n");
    
    int total_failures = 0;
    
    // Run functional tests
    printf("Running functional tests...\n");
    total_failures += run_game_object_tests();
    printf("\n");
    
    // Run performance tests
    printf("Running performance tests...\n");
    total_failures += run_gameobject_performance_tests();
    printf("\n");
    
    if (total_failures == 0) {
        printf("🎉 All GameObject tests passed!\n");
        printf("Phase 3: GameObject & Transform implementation is complete.\n");
    } else {
        printf("❌ %d tests failed\n", total_failures);
    }
    
    return total_failures;
}