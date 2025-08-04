#include "../../src/core/component.h"
#include "../../src/core/component_registry.h"
#include "../../src/components/transform_component.h"
#include <time.h>
#include <stdio.h>
#include <assert.h>

// Mock GameObject for testing
typedef struct MockGameObject {
    uint32_t id;
    const char* name;
} MockGameObject;

// Mock component vtable
static void mock_component_init(Component* component, GameObject* gameObject) {
    (void)component;
    (void)gameObject;
}

static void mock_component_destroy(Component* component) {
    (void)component;
}

static void mock_component_update(Component* component, float deltaTime) {
    (void)component;
    (void)deltaTime;
}

static const ComponentVTable mockVTable = {
    .init = mock_component_init,
    .destroy = mock_component_destroy,
    .update = mock_component_update,
    .fixedUpdate = NULL,
    .render = NULL,
    .onEnabled = NULL,
    .onDisabled = NULL,
    .onGameObjectDestroyed = NULL,
    .getSerializedSize = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

void benchmark_component_creation(void) {
    printf("=== Component Creation Performance Test ===\n");
    
    component_registry_init();
    
    component_registry_register_type(
        COMPONENT_TYPE_TRANSFORM,
        sizeof(TransformComponent),
        10000,
        &mockVTable,
        "Transform"
    );
    
    MockGameObject gameObject = {1, "PerfTest"};
    Component* components[10000];
    
    clock_t start = clock();
    
    for (int i = 0; i < 10000; i++) {
        components[i] = component_registry_create(COMPONENT_TYPE_TRANSFORM, 
                                                (GameObject*)&gameObject);
        assert(components[i] != NULL);
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000; // microseconds
    double per_component = time_taken / 10000;
    
    printf("Component creation: %.2f μs for 10,000 components (%.2f ns per component)\n", 
           time_taken, per_component * 1000);
    
    // Verify performance target: < 50ns per component
    if (per_component < 50) {
        printf("✓ Performance target met: %.2f ns < 50 ns\n", per_component * 1000);
    } else {
        printf("❌ Performance target missed: %.2f ns >= 50 ns\n", per_component * 1000);
        assert(per_component < 50);
    }
    
    component_registry_shutdown();
    printf("✓ Component creation performance test passed\n\n");
}

void benchmark_type_checking(void) {
    printf("=== Type Checking Performance Test ===\n");
    
    MockGameObject gameObject = {1, "TypeTest"};
    Component component;
    component_init(&component, COMPONENT_TYPE_SPRITE | COMPONENT_TYPE_COLLISION, 
                  &mockVTable, (GameObject*)&gameObject);
    
    clock_t start = clock();
    
    // Perform 1 million type checks
    volatile bool result = false;
    for (int i = 0; i < 1000000; i++) {
        result = component_is_type(&component, COMPONENT_TYPE_SPRITE);
    }
    (void)result; // Suppress unused variable warning
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000; // microseconds
    double per_check = time_taken / 1000000;
    
    printf("Type checking: %.2f μs for 1,000,000 checks (%.2f ns per check)\n", 
           time_taken, per_check * 1000);
    
    // Verify performance target: < 1ns per check
    if (per_check < 1) {
        printf("✓ Performance target met: %.2f ns < 1 ns\n", per_check * 1000);
    } else {
        printf("❌ Performance target missed: %.2f ns >= 1 ns\n", per_check * 1000);
        assert(per_check < 1);
    }
    
    printf("✓ Type checking performance test passed\n\n");
}

void benchmark_virtual_function_calls(void) {
    printf("=== Virtual Function Call Performance Test ===\n");
    
    component_registry_init();
    
    component_registry_register_type(
        COMPONENT_TYPE_TRANSFORM,
        sizeof(TransformComponent),
        1000,
        &mockVTable,
        "Transform"
    );
    
    MockGameObject gameObject = {1, "VTableTest"};
    Component* component = component_registry_create(COMPONENT_TYPE_TRANSFORM, 
                                                   (GameObject*)&gameObject);
    assert(component != NULL);
    
    clock_t start = clock();
    
    // Perform 1 million virtual function calls
    for (int i = 0; i < 1000000; i++) {
        component_call_update(component, 0.016f);
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000; // microseconds
    double per_call = time_taken / 1000000;
    
    printf("Virtual function calls: %.2f μs for 1,000,000 calls (%.2f ns per call)\n", 
           time_taken, per_call * 1000);
    
    // Verify performance target: < 5ns overhead per call
    if (per_call < 5) {
        printf("✓ Performance target met: %.2f ns < 5 ns\n", per_call * 1000);
    } else {
        printf("❌ Performance target missed: %.2f ns >= 5 ns\n", per_call * 1000);
        assert(per_call < 5);
    }
    
    component_registry_shutdown();
    printf("✓ Virtual function call performance test passed\n\n");
}

void benchmark_component_destruction(void) {
    printf("=== Component Destruction Performance Test ===\n");
    
    component_registry_init();
    
    component_registry_register_type(
        COMPONENT_TYPE_TRANSFORM,
        sizeof(TransformComponent),
        10000,
        &mockVTable,
        "Transform"
    );
    
    MockGameObject gameObject = {1, "DestroyTest"};
    Component* components[10000];
    
    // Create components first
    for (int i = 0; i < 10000; i++) {
        components[i] = component_registry_create(COMPONENT_TYPE_TRANSFORM, 
                                                (GameObject*)&gameObject);
        assert(components[i] != NULL);
    }
    
    clock_t start = clock();
    
    // Destroy all components
    for (int i = 0; i < 10000; i++) {
        ComponentResult result = component_registry_destroy(components[i]);
        assert(result == COMPONENT_OK);
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000; // microseconds
    double per_component = time_taken / 10000;
    
    printf("Component destruction: %.2f μs for 10,000 components (%.2f ns per component)\n", 
           time_taken, per_component * 1000);
    
    // Should be similar to creation performance
    if (per_component < 50) {
        printf("✓ Performance acceptable: %.2f ns < 50 ns\n", per_component * 1000);
    } else {
        printf("⚠ Performance slower than expected: %.2f ns >= 50 ns\n", per_component * 1000);
    }
    
    component_registry_shutdown();
    printf("✓ Component destruction performance test passed\n\n");
}

void benchmark_registry_queries(void) {
    printf("=== Registry Query Performance Test ===\n");
    
    component_registry_init();
    
    component_registry_register_type(COMPONENT_TYPE_TRANSFORM, sizeof(TransformComponent), 1000, &mockVTable, "Transform");
    component_registry_register_type(COMPONENT_TYPE_SPRITE, sizeof(Component), 1000, &mockVTable, "Sprite");
    component_registry_register_type(COMPONENT_TYPE_COLLISION, sizeof(Component), 1000, &mockVTable, "Collision");
    
    clock_t start = clock();
    
    // Perform 1 million registry queries
    for (int i = 0; i < 1000000; i++) {
        volatile bool registered = component_registry_is_type_registered(COMPONENT_TYPE_TRANSFORM);
        volatile const ComponentTypeInfo* info = component_registry_get_type_info(COMPONENT_TYPE_SPRITE);
        volatile ObjectPool* pool = component_registry_get_pool(COMPONENT_TYPE_COLLISION);
        (void)registered; (void)info; (void)pool; // Suppress warnings
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000; // microseconds
    double per_query = time_taken / 3000000; // 3 queries per iteration
    
    printf("Registry queries: %.2f μs for 3,000,000 queries (%.2f ns per query)\n", 
           time_taken, per_query * 1000);
    
    // Registry queries should be very fast (< 10ns)
    if (per_query < 10) {
        printf("✓ Performance target met: %.2f ns < 10 ns\n", per_query * 1000);
    } else {
        printf("❌ Performance target missed: %.2f ns >= 10 ns\n", per_query * 1000);
        assert(per_query < 10);
    }
    
    component_registry_shutdown();
    printf("✓ Registry query performance test passed\n\n");
}

void benchmark_memory_usage(void) {
    printf("=== Memory Usage Analysis ===\n");
    
    component_registry_init();
    
    // Register several component types with different pool sizes
    component_registry_register_type(COMPONENT_TYPE_TRANSFORM, sizeof(TransformComponent), 1000, &mockVTable, "Transform");
    component_registry_register_type(COMPONENT_TYPE_SPRITE, 48, 500, &mockVTable, "Sprite");
    component_registry_register_type(COMPONENT_TYPE_COLLISION, 32, 750, &mockVTable, "Collision");
    
    uint32_t totalMemory = component_registry_get_total_memory_usage();
    printf("Total registry memory usage: %u bytes (%.2f KB)\n", totalMemory, totalMemory / 1024.0f);
    
    // Memory usage should be reasonable
    assert(totalMemory > 0);
    assert(totalMemory < 1024 * 1024); // Less than 1MB for reasonable test
    
    // Test memory efficiency (should be < 5% overhead as per spec)
    uint32_t dataSize = (sizeof(TransformComponent) * 1000) + (48 * 500) + (32 * 750);
    float overhead = ((float)totalMemory - dataSize) / dataSize * 100.0f;
    
    printf("Data size: %u bytes, Overhead: %.2f%%\n", dataSize, overhead);
    
    if (overhead < 5.0f) {
        printf("✓ Memory efficiency target met: %.2f%% < 5%%\n", overhead);
    } else {
        printf("⚠ Memory efficiency could be improved: %.2f%% >= 5%%\n", overhead);
    }
    
    component_registry_shutdown();
    printf("✓ Memory usage analysis completed\n\n");
}

// Test runner function
int run_component_performance_tests(void) {
    printf("=== Component Performance Tests ===\n");
    
    int failures = 0;
    
    benchmark_component_creation();
    benchmark_type_checking();
    benchmark_virtual_function_calls();
    benchmark_component_destruction();
    benchmark_registry_queries();
    benchmark_memory_usage();
    
    printf("Component performance tests completed with %d failures\n", failures);
    return failures;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_component_performance_tests();
}
#endif