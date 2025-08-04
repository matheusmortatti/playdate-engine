#include "../../src/core/memory_pool.h"
#include "../../src/core/memory_debug.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct DebugTestObject {
    int id;
    char data[60]; // 64 bytes total
} DebugTestObject;

void test_debug_initialization(void) {
    memory_debug_init();
    
    MemoryStats stats = memory_debug_get_stats();
    assert(stats.totalPools == 0);
    assert(stats.totalAllocatedObjects == 0);
    assert(stats.totalMemoryUsed == 0);
    assert(stats.totalAllocations == 0);
    assert(stats.totalDeallocations == 0);
    
    memory_debug_shutdown();
    printf("✓ Debug initialization test passed\n");
}

void test_pool_registration(void) {
    memory_debug_init();
    
    ObjectPool pool1, pool2;
    object_pool_init(&pool1, sizeof(DebugTestObject), 10, "TestPool1");
    object_pool_init(&pool2, sizeof(DebugTestObject), 20, "TestPool2");
    
    // Register pools
    memory_debug_register_pool(&pool1);
    memory_debug_register_pool(&pool2);
    
    MemoryStats stats = memory_debug_get_stats();
    assert(stats.totalPools == 2);
    
    // Unregister one pool
    memory_debug_unregister_pool(&pool1);
    stats = memory_debug_get_stats();
    assert(stats.totalPools == 1);
    
    // Clean up
    memory_debug_unregister_pool(&pool2);
    object_pool_destroy(&pool1);
    object_pool_destroy(&pool2);
    
    memory_debug_shutdown();
    printf("✓ Pool registration test passed\n");
}

void test_debug_statistics_tracking(void) {
    memory_debug_init();
    
    ObjectPool pool;
    object_pool_init(&pool, sizeof(DebugTestObject), 5, "StatsPool");
    memory_debug_register_pool(&pool);
    
    // Initial state
    MemoryStats stats = memory_debug_get_stats();
    assert(stats.totalPools == 1);
    assert(stats.totalAllocatedObjects == 0);
    
    // Allocate some objects
    DebugTestObject* objects[3];
    for (int i = 0; i < 3; i++) {
        objects[i] = (DebugTestObject*)object_pool_alloc(&pool);
        assert(objects[i] != NULL);
        objects[i]->id = i;
    }
    
    stats = memory_debug_get_stats();
    assert(stats.totalPools == 1);
    assert(stats.totalAllocatedObjects == 3);
    assert(stats.totalAllocations == 3);
    assert(stats.totalDeallocations == 0);
    assert(stats.totalMemoryUsed == 3 * pool.elementSize);
    
    // Free one object
    object_pool_free(&pool, objects[1]);
    
    stats = memory_debug_get_stats();
    assert(stats.totalAllocatedObjects == 2);
    assert(stats.totalAllocations == 3);
    assert(stats.totalDeallocations == 1);
    assert(stats.totalMemoryUsed == 2 * pool.elementSize);
    
    // Clean up
    memory_debug_unregister_pool(&pool);
    object_pool_destroy(&pool);
    memory_debug_shutdown();
    printf("✓ Statistics tracking test passed\n");
}

void test_snapshot_functionality(void) {
    memory_debug_init();
    
    ObjectPool pool;
    object_pool_init(&pool, sizeof(DebugTestObject), 10, "SnapshotPool");
    memory_debug_register_pool(&pool);
    
    // Take initial snapshot
    memory_debug_snapshot();
    
    // Allocate some objects
    DebugTestObject* objects[5];
    for (int i = 0; i < 5; i++) {
        objects[i] = (DebugTestObject*)object_pool_alloc(&pool);
        assert(objects[i] != NULL);
    }
    
    // Compare snapshots - should show 5 new objects
    printf("Expected to show 5 new objects:\n");
    memory_debug_compare_snapshots();
    
    // Take another snapshot
    memory_debug_snapshot();
    
    // Free 2 objects
    object_pool_free(&pool, objects[0]);
    object_pool_free(&pool, objects[1]);
    
    // Compare again - should show 2 objects freed
    printf("Expected to show 2 objects freed:\n");
    memory_debug_compare_snapshots();
    
    // Clean up
    memory_debug_unregister_pool(&pool);
    object_pool_destroy(&pool);
    memory_debug_shutdown();
    printf("✓ Snapshot functionality test passed\n");
}

void test_leak_detection(void) {
    memory_debug_init();
    
    ObjectPool pool;
    object_pool_init(&pool, sizeof(DebugTestObject), 5, "LeakPool");
    memory_debug_register_pool(&pool);
    
    // Take snapshot
    memory_debug_snapshot();
    
    // Allocate objects and "forget" to free them
    DebugTestObject* obj1 = (DebugTestObject*)object_pool_alloc(&pool);
    DebugTestObject* obj2 = (DebugTestObject*)object_pool_alloc(&pool);
    assert(obj1 != NULL && obj2 != NULL);
    
    printf("Expected to detect 2 potential leaks:\n");
    memory_debug_compare_snapshots();
    
    // Free objects to clean up
    object_pool_free(&pool, obj1);
    object_pool_free(&pool, obj2);
    
    // Clean up
    memory_debug_unregister_pool(&pool);
    object_pool_destroy(&pool);
    memory_debug_shutdown();
    printf("✓ Leak detection test passed\n");
}

void test_multiple_pools_tracking(void) {
    memory_debug_init();
    
    ObjectPool pool1, pool2, pool3;
    object_pool_init(&pool1, 32, 10, "SmallPool");
    object_pool_init(&pool2, 64, 20, "MediumPool");
    object_pool_init(&pool3, 128, 5, "LargePool");
    
    memory_debug_register_pool(&pool1);
    memory_debug_register_pool(&pool2);
    memory_debug_register_pool(&pool3);
    
    // Allocate from different pools
    void* obj1 = object_pool_alloc(&pool1);
    void* obj2a = object_pool_alloc(&pool2);
    void* obj2b = object_pool_alloc(&pool2);
    void* obj3 = object_pool_alloc(&pool3);
    
    assert(obj1 != NULL && obj2a != NULL && obj2b != NULL && obj3 != NULL);
    
    MemoryStats stats = memory_debug_get_stats();
    assert(stats.totalPools == 3);
    assert(stats.totalAllocatedObjects == 4);
    
    uint32_t expectedMemory = pool1.elementSize + 2 * pool2.elementSize + pool3.elementSize;
    assert(stats.totalMemoryUsed == expectedMemory);
    
    // Print detailed report
    printf("Multi-pool report:\n");
    memory_debug_print_report();
    
    // Clean up
    memory_debug_unregister_pool(&pool1);
    memory_debug_unregister_pool(&pool2);  
    memory_debug_unregister_pool(&pool3);
    object_pool_destroy(&pool1);
    object_pool_destroy(&pool2);
    object_pool_destroy(&pool3);
    
    memory_debug_shutdown();
    printf("✓ Multiple pools tracking test passed\n");
}

void test_error_conditions_debug(void) {
    memory_debug_init();
    
    // Test registering NULL pool
    memory_debug_register_pool(NULL);
    
    // Test unregistering NULL pool
    memory_debug_unregister_pool(NULL);
    
    // Test unregistering unknown pool
    ObjectPool pool;
    object_pool_init(&pool, sizeof(DebugTestObject), 5, "UnknownPool");
    memory_debug_unregister_pool(&pool); // Should print warning
    
    object_pool_destroy(&pool);
    memory_debug_shutdown();
    printf("✓ Debug error conditions test passed\n");
}

int run_memory_debug_tests(void) {
    printf("Running memory debug tests...\n");
    
    test_debug_initialization();
    test_pool_registration();
    test_debug_statistics_tracking();
    test_snapshot_functionality();
    test_leak_detection();
    test_multiple_pools_tracking();
    test_error_conditions_debug();
    
    printf("All memory debug tests passed! ✓\n\n");
    return 0;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_memory_debug_tests();
}
#endif