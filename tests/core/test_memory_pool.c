#include "../../src/core/memory_pool.h"
#include "../../src/core/memory_debug.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct TestObject {
    int value;
    float position[3];
    char padding[4]; // Ensure non-aligned size for testing
} TestObject;

void test_pool_initialization(void) {
    ObjectPool pool;
    PoolResult result = object_pool_init(&pool, sizeof(TestObject), 100, "TestPool");
    
    assert(result == POOL_OK);
    assert(pool.capacity == 100);
    assert(pool.freeCount == 100);
    assert(pool.elementSize >= sizeof(TestObject)); // May be aligned larger
    assert(pool.elementSize % MEMORY_ALIGNMENT == 0); // Must be aligned
    assert(pool.memory != NULL);
    assert(pool.freeList != NULL);
    assert(pool.objectStates != NULL);
    assert(pool.debugName != NULL);
    assert(strcmp(pool.debugName, "TestPool") == 0);
    
    // Check statistics initialization
    assert(pool.totalAllocations == 0);
    assert(pool.totalDeallocations == 0);
    assert(pool.peakUsage == 0);
    
    object_pool_destroy(&pool);
    printf("✓ Pool initialization test passed\n");
}

void test_allocation_deallocation(void) {
    ObjectPool pool;
    object_pool_init(&pool, sizeof(TestObject), 10, "AllocTest");
    
    TestObject* objects[10];
    
    // Allocate all objects
    for (int i = 0; i < 10; i++) {
        objects[i] = (TestObject*)object_pool_alloc(&pool);
        assert(objects[i] != NULL);
        assert(object_pool_owns_object(&pool, objects[i]));
        objects[i]->value = i;
    }
    
    assert(object_pool_get_free_count(&pool) == 0);
    assert(object_pool_get_used_count(&pool) == 10);
    assert(object_pool_get_usage_percent(&pool) == 100.0f);
    assert(pool.totalAllocations == 10);
    assert(pool.peakUsage == 10);
    
    // Pool should be full
    TestObject* extraObject = (TestObject*)object_pool_alloc(&pool);
    assert(extraObject == NULL);
    
    // Verify data integrity
    for (int i = 0; i < 10; i++) {
        assert(objects[i]->value == i);
    }
    
    // Free half the objects
    for (int i = 0; i < 5; i++) {
        PoolResult result = object_pool_free(&pool, objects[i]);
        assert(result == POOL_OK);
    }
    
    assert(object_pool_get_free_count(&pool) == 5);
    assert(object_pool_get_used_count(&pool) == 5);
    assert(object_pool_get_usage_percent(&pool) == 50.0f);
    assert(pool.totalDeallocations == 5);
    
    // Should be able to allocate again
    TestObject* newObject = (TestObject*)object_pool_alloc(&pool);
    assert(newObject != NULL);
    assert(object_pool_owns_object(&pool, newObject));
    
    object_pool_destroy(&pool);
    printf("✓ Allocation/deallocation test passed\n");
}

void test_alignment(void) {
    ObjectPool pool;
    object_pool_init(&pool, sizeof(TestObject), 10, "AlignTest");
    
    void* objects[10];
    
    // Allocate all objects and check alignment
    for (int i = 0; i < 10; i++) {
        objects[i] = object_pool_alloc(&pool);
        assert(objects[i] != NULL);
        
        // Check alignment
        uintptr_t addr = (uintptr_t)objects[i];
        assert(addr % MEMORY_ALIGNMENT == 0);
    }
    
    // Verify objects are properly spaced
    for (int i = 1; i < 10; i++) {
        uintptr_t addr1 = (uintptr_t)objects[i-1];
        uintptr_t addr2 = (uintptr_t)objects[i];
        uintptr_t diff = (addr2 > addr1) ? (addr2 - addr1) : (addr1 - addr2);
        assert(diff >= pool.elementSize);
    }
    
    object_pool_destroy(&pool);
    printf("✓ Memory alignment test passed\n");
}

void test_error_conditions(void) {
    ObjectPool pool;
    
    // Test null pointer errors
    assert(object_pool_init(NULL, sizeof(TestObject), 10, "Test") == POOL_ERROR_NULL_POINTER);
    assert(object_pool_init(&pool, 0, 10, "Test") == POOL_ERROR_NULL_POINTER);
    assert(object_pool_init(&pool, sizeof(TestObject), 0, "Test") == POOL_ERROR_NULL_POINTER);
    
    // Initialize valid pool
    PoolResult result = object_pool_init(&pool, sizeof(TestObject), 5, "ErrorTest");
    assert(result == POOL_OK);
    
    // Test double free detection
    TestObject* obj = (TestObject*)object_pool_alloc(&pool);
    assert(obj != NULL);
    
    assert(object_pool_free(&pool, obj) == POOL_OK);
    assert(object_pool_free(&pool, obj) == POOL_ERROR_DOUBLE_FREE);
    
    // Test invalid object
    TestObject invalidObj;
    assert(object_pool_free(&pool, &invalidObj) == POOL_ERROR_INVALID_INDEX);
    assert(!object_pool_owns_object(&pool, &invalidObj));
    
    // Test null operations
    assert(object_pool_alloc(NULL) == NULL);
    assert(object_pool_free(NULL, obj) == POOL_ERROR_NULL_POINTER);
    assert(object_pool_free(&pool, NULL) == POOL_ERROR_NULL_POINTER);
    
    object_pool_destroy(&pool);
    printf("✓ Error conditions test passed\n");
}

void test_object_ownership(void) {
    ObjectPool pool1, pool2;
    object_pool_init(&pool1, sizeof(TestObject), 5, "Pool1");
    object_pool_init(&pool2, sizeof(TestObject), 5, "Pool2");
    
    TestObject* obj1 = (TestObject*)object_pool_alloc(&pool1);
    TestObject* obj2 = (TestObject*)object_pool_alloc(&pool2);
    
    assert(obj1 != NULL && obj2 != NULL);
    
    // Test ownership
    assert(object_pool_owns_object(&pool1, obj1));
    assert(!object_pool_owns_object(&pool1, obj2));
    assert(object_pool_owns_object(&pool2, obj2));
    assert(!object_pool_owns_object(&pool2, obj1));
    
    // Test indices
    uint32_t index1 = object_pool_get_object_index(&pool1, obj1);
    uint32_t index2 = object_pool_get_object_index(&pool2, obj2);
    assert(index1 != UINT32_MAX);
    assert(index2 != UINT32_MAX);
    
    // Cross-pool operations should fail
    assert(object_pool_free(&pool1, obj2) == POOL_ERROR_INVALID_INDEX);
    assert(object_pool_free(&pool2, obj1) == POOL_ERROR_INVALID_INDEX);
    
    object_pool_destroy(&pool1);
    object_pool_destroy(&pool2);
    printf("✓ Object ownership test passed\n");
}

void test_statistics_tracking(void) {
    ObjectPool pool;
    object_pool_init(&pool, sizeof(TestObject), 10, "StatsTest");
    
    assert(pool.totalAllocations == 0);
    assert(pool.totalDeallocations == 0);
    assert(pool.peakUsage == 0);
    
    TestObject* objects[5];
    
    // Allocate some objects
    for (int i = 0; i < 5; i++) {
        objects[i] = (TestObject*)object_pool_alloc(&pool);
        assert(pool.totalAllocations == (uint32_t)(i + 1));
        assert(pool.peakUsage == (uint32_t)(i + 1));
    }
    
    // Free some objects
    for (int i = 0; i < 3; i++) {
        object_pool_free(&pool, objects[i]);
        assert(pool.totalDeallocations == (uint32_t)(i + 1));
    }
    
    assert(pool.peakUsage == 5); // Should remain at peak
    assert(object_pool_get_used_count(&pool) == 2);
    
    // Allocate more to test peak tracking
    for (int i = 0; i < 3; i++) {
        TestObject* obj = (TestObject*)object_pool_alloc(&pool);
        assert(obj != NULL);
    }
    
    assert(pool.peakUsage == 5); // Peak should not exceed capacity
    assert(object_pool_get_used_count(&pool) == 5);
    
    object_pool_destroy(&pool);
    printf("✓ Statistics tracking test passed\n");
}

int run_memory_pool_tests(void) {
    printf("Running memory pool tests...\n");
    
    test_pool_initialization();
    test_allocation_deallocation();
    test_alignment();
    test_error_conditions();
    test_object_ownership();
    test_statistics_tracking();
    
    printf("All memory pool tests passed! ✓\n\n");
    return 0;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_memory_pool_tests();
}
#endif