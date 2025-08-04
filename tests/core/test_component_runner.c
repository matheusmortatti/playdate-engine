#include "../../src/core/component.h"
#include "../../src/core/component_registry.h"
#include "../../src/core/memory_pool.h"
#include <stdio.h>

// External test function declarations
extern int run_component_tests(void);
extern int run_component_registry_tests(void);
extern int run_component_performance_tests(void);
extern int run_transform_component_tests(void);
extern int run_component_factory_tests(void);

int main(void) {
    printf("=== Playdate Engine Component System Test Suite ===\n\n");
    
    int total_failures = 0;
    
    // Run unit tests
    printf("PHASE 1: Core Component Tests\n");
    printf("==============================\n");
    total_failures += run_component_tests();
    printf("\n");
    
    printf("PHASE 2: Component Registry Tests\n");
    printf("==================================\n");
    total_failures += run_component_registry_tests();
    printf("\n");
    
    printf("PHASE 3: Transform Component Tests\n");
    printf("===================================\n");
    total_failures += run_transform_component_tests();
    printf("\n");
    
    printf("PHASE 4: Component Factory Tests\n");
    printf("=================================\n");
    total_failures += run_component_factory_tests();
    printf("\n");
    
    printf("PHASE 5: Performance Tests\n");
    printf("===========================\n");
    total_failures += run_component_performance_tests();
    printf("\n");
    
    // Summary
    printf("=== Test Suite Summary ===\n");
    if (total_failures == 0) {
        printf("🎉 ALL TESTS PASSED! 🎉\n");
        printf("Component system is ready for Phase 3.\n");
    } else {
        printf("❌ %d test(s) failed\n", total_failures);
        printf("Please fix issues before proceeding to Phase 3.\n");
    }
    printf("===========================\n\n");
    
    return total_failures;
}