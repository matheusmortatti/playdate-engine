#include "../../src/core/game_object.h"
#include "../../src/components/transform_component.h"
#include "../../src/core/component_registry.h"
#include "mock_scene.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define PERFORMANCE_ITERATIONS 5000
#define LARGE_SCENE_SIZE 10000

// Utility function to get time in nanoseconds
uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void benchmark_gameobject_creation(void) {
    component_registry_init();
    
    MockScene* scene = mock_scene_create_with_capacity(PERFORMANCE_ITERATIONS, "PerfTestScene");
    GameObject* gameObjects[PERFORMANCE_ITERATIONS];
    
    printf("Benchmarking GameObject creation (%d objects)...\n", PERFORMANCE_ITERATIONS);
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < PERFORMANCE_ITERATIONS; i++) {
        gameObjects[i] = game_object_create((Scene*)scene);
        if (!gameObjects[i]) {
            printf("Failed to create GameObject at index %d\n", i);
            printf("Scene capacity: %u, current count: %u\n", 
                   mock_scene_get_max_gameobjects(scene), 
                   mock_scene_get_gameobject_count(scene));
            break;
        }
    }
    
    uint64_t end = get_time_ns();
    
    uint64_t total_time_ns = end - start;
    double time_per_object_ns = (double)total_time_ns / PERFORMANCE_ITERATIONS;
    double time_per_object_us = time_per_object_ns / 1000.0;
    
    printf("GameObject creation: %.2f ns per object (%.2f μs total)\n", 
           time_per_object_ns, time_per_object_us);
    
    // Verify performance target: < 100ns per GameObject
    if (time_per_object_ns < 100.0) {
        printf("✓ GameObject creation performance target met (< 100ns)\n");
    } else {
        printf("✗ GameObject creation performance target missed: %.2f ns > 100ns\n", time_per_object_ns);
    }
    
    // Cleanup
    for (int i = 0; i < PERFORMANCE_ITERATIONS; i++) {
        game_object_destroy(gameObjects[i]);
    }
    
    mock_scene_destroy(scene);
    component_registry_shutdown();
}

void benchmark_component_queries(void) {
    component_registry_init();
    
    MockScene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create((Scene*)scene);
    
    printf("Benchmarking component queries (1,000,000 checks)...\n");
    
    const int iterations = 1000000;
    uint64_t start = get_time_ns();
    
    // Perform 1 million component checks
    volatile bool result = false;
    for (int i = 0; i < iterations; i++) {
        result = game_object_has_component(gameObject, COMPONENT_TYPE_TRANSFORM);
    }
    (void)result; // Prevent optimization
    
    uint64_t end = get_time_ns();
    
    uint64_t total_time_ns = end - start;
    double time_per_check_ns = (double)total_time_ns / iterations;
    
    printf("Component queries: %.3f ns per check\n", time_per_check_ns);
    
    // Verify performance target: < 1ns per check (bitmask operation)
    if (time_per_check_ns < 1.0) {
        printf("✓ Component query performance target met (< 1ns)\n");
    } else {
        printf("✗ Component query performance target missed: %.3f ns > 1ns\n", time_per_check_ns);
    }
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
}

void benchmark_transform_updates(void) {
    component_registry_init();
    
    MockScene* scene = mock_scene_create_with_capacity(1000, "TransformPerfScene");
    GameObject* gameObjects[1000];
    
    // Create many GameObjects
    for (int i = 0; i < 1000; i++) {
        gameObjects[i] = game_object_create((Scene*)scene);
        assert(gameObjects[i] != NULL);
    }
    
    printf("Benchmarking transform updates (1000 objects, 100 frames)...\n");
    
    uint64_t start = get_time_ns();
    
    // Update all transforms for 100 frames
    for (int frame = 0; frame < 100; frame++) {
        for (int i = 0; i < 1000; i++) {
            game_object_translate(gameObjects[i], 0.1f, 0.1f);
        }
    }
    
    uint64_t end = get_time_ns();
    
    uint64_t total_time_ns = end - start;
    double time_per_frame_ms = (double)total_time_ns / (100 * 1000000.0); // Convert to milliseconds
    double time_per_object_ns = (double)total_time_ns / (100 * 1000); // ns per object per frame
    
    printf("Transform updates: %.2f ms per frame, %.2f ns per object\n", 
           time_per_frame_ms, time_per_object_ns);
    
    // Verify performance targets
    bool frame_target_met = time_per_frame_ms < 1.0; // < 1ms per frame for 1000 objects
    bool object_target_met = time_per_object_ns < 20.0; // < 20ns per object
    
    if (frame_target_met) {
        printf("✓ Frame update performance target met (< 1ms per frame)\n");
    } else {
        printf("✗ Frame update performance target missed: %.2f ms > 1ms\n", time_per_frame_ms);
    }
    
    if (object_target_met) {
        printf("✓ Object update performance target met (< 20ns per object)\n");
    } else {
        printf("✗ Object update performance target missed: %.2f ns > 20ns\n", time_per_object_ns);
    }
    
    // Cleanup
    for (int i = 0; i < 1000; i++) {
        game_object_destroy(gameObjects[i]);
    }
    
    mock_scene_destroy(scene);
    component_registry_shutdown();
}

void benchmark_large_scene(void) {
    component_registry_init();
    
    printf("Benchmarking large scene support (%d GameObjects)...\n", LARGE_SCENE_SIZE);
    
    MockScene* scene = mock_scene_create_with_capacity(LARGE_SCENE_SIZE, "LargeScene");
    if (!scene) {
        printf("✗ Failed to create large scene\n");
        return;
    }
    
    uint64_t creation_start = get_time_ns();
    
    // Create large number of GameObjects
    GameObject** gameObjects = malloc(LARGE_SCENE_SIZE * sizeof(GameObject*));
    if (!gameObjects) {
        printf("✗ Failed to allocate memory for large scene test\n");
        mock_scene_destroy(scene);
        component_registry_shutdown();
        return;
    }
    
    bool creation_success = true;
    for (int i = 0; i < LARGE_SCENE_SIZE; i++) {
        gameObjects[i] = game_object_create((Scene*)scene);
        if (!gameObjects[i]) {
            printf("✗ Failed to create GameObject %d\n", i);
            creation_success = false;
            break;
        }
        
        // Set some varying positions to test transform system
        game_object_set_position(gameObjects[i], (float)(i % 1000), (float)(i / 1000));
    }
    
    uint64_t creation_end = get_time_ns();
    
    if (creation_success) {
        double creation_time_ms = (double)(creation_end - creation_start) / 1000000.0;
        printf("✓ Successfully created %d GameObjects in %.2f ms\n", 
               LARGE_SCENE_SIZE, creation_time_ms);
        
        // Test query performance on large scene
        uint64_t query_start = get_time_ns();
        int active_count = 0;
        for (int i = 0; i < LARGE_SCENE_SIZE; i++) {
            if (game_object_is_active(gameObjects[i])) {
                active_count++;
            }
        }
        uint64_t query_end = get_time_ns();
        
        double query_time_ms = (double)(query_end - query_start) / 1000000.0;
        printf("✓ Queried %d active GameObjects in %.2f ms\n", active_count, query_time_ms);
        
        // Test batch transform updates
        uint64_t update_start = get_time_ns();
        for (int i = 0; i < LARGE_SCENE_SIZE; i++) {
            game_object_translate(gameObjects[i], 0.01f, 0.01f);
        }
        uint64_t update_end = get_time_ns();
        
        double update_time_ms = (double)(update_end - update_start) / 1000000.0;
        printf("✓ Updated %d GameObjects transforms in %.2f ms\n", 
               LARGE_SCENE_SIZE, update_time_ms);
        
        // Cleanup all objects
        uint64_t cleanup_start = get_time_ns();
        for (int i = 0; i < LARGE_SCENE_SIZE; i++) {
            if (gameObjects[i]) {
                game_object_destroy(gameObjects[i]);
            }
        }
        uint64_t cleanup_end = get_time_ns();
        
        double cleanup_time_ms = (double)(cleanup_end - cleanup_start) / 1000000.0;
        printf("✓ Cleaned up %d GameObjects in %.2f ms\n", 
               LARGE_SCENE_SIZE, cleanup_time_ms);
    } else {
        // Cleanup partial creation
        for (int i = 0; i < LARGE_SCENE_SIZE; i++) {
            if (gameObjects[i]) {
                game_object_destroy(gameObjects[i]);
            }
        }
    }
    
    free(gameObjects);
    mock_scene_destroy(scene);
    component_registry_shutdown();
}

void benchmark_memory_usage(void) {
    component_registry_init();
    
    printf("Analyzing memory usage...\n");
    
    // Verify structure sizes match specification
    printf("Structure sizes:\n");
    printf("  GameObject: %zu bytes (target: 64)\n", sizeof(GameObject));
    printf("  TransformComponent: %zu bytes (target: 64)\n", sizeof(TransformComponent));
    printf("  Component (base): %zu bytes\n", sizeof(Component));
    
    // Verify alignment
    printf("Alignment verification:\n");
    printf("  GameObject aligned to 16 bytes: %s\n", 
           (sizeof(GameObject) % 16 == 0) ? "✓" : "✗");
    printf("  TransformComponent aligned to 16 bytes: %s\n", 
           (sizeof(TransformComponent) % 16 == 0) ? "✓" : "✗");
    
    // Test memory pool usage
    MockScene* scene = mock_scene_create_with_capacity(100, "MemoryTestScene");
    
    printf("Object pool efficiency:\n");
    printf("  Pool capacity: %u objects\n", mock_scene_get_max_gameobjects(scene));
    printf("  Initial usage: %.1f%%\n", mock_scene_get_usage_percent(scene));
    
    // Create some objects
    GameObject* objects[10];
    for (int i = 0; i < 10; i++) {
        objects[i] = game_object_create((Scene*)scene);
    }
    
    printf("  After creating 10 objects: %.1f%%\n", mock_scene_get_usage_percent(scene));
    
    // Cleanup
    for (int i = 0; i < 10; i++) {
        game_object_destroy(objects[i]);
    }
    
    printf("  After cleanup: %.1f%%\n", mock_scene_get_usage_percent(scene));
    
    mock_scene_destroy(scene);
    component_registry_shutdown();
}

// Performance test runner
int run_gameobject_performance_tests(void) {
    printf("=== GameObject Performance Tests ===\n");
    
    benchmark_gameobject_creation();
    printf("\n");
    
    benchmark_component_queries();
    printf("\n");
    
    benchmark_transform_updates();
    printf("\n");
    
    benchmark_large_scene();
    printf("\n");
    
    benchmark_memory_usage();
    printf("\n");
    
    printf("Performance tests completed\n");
    return 0;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_gameobject_performance_tests();
}
#endif