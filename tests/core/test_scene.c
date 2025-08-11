#include "../../src/core/scene.h"
#include "../../src/core/game_object.h" 
#include "../../src/core/component_registry.h"
#include "../../src/components/transform_component.h"
#include "../../src/core/update_systems.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Declare external functions for transform component testing
extern ComponentResult transform_component_register(void);

void test_scene_creation_destruction(void) {
    Scene* scene = scene_create("TestScene", 100);
    
    // Test that creation returns non-null
    assert(scene != NULL);
    assert(strcmp(scene->name, "TestScene") == 0);
    assert(scene->state == SCENE_STATE_INACTIVE);
    assert(scene->gameObjectCount == 0);
    assert(scene->gameObjectCapacity == 100);
    assert(scene->timeScale == 1.0f);
    assert(scene->totalTime == 0.0f);
    assert(scene->frameCount == 0);
    assert(scene->activeObjectCount == 0);
    
    // Test arrays are allocated
    assert(scene->gameObjects != NULL);
    assert(scene->rootObjects != NULL);
    assert(scene->transformComponents != NULL);
    assert(scene->spriteComponents != NULL);
    assert(scene->collisionComponents != NULL);
    
    // Test component counts start at zero
    assert(scene->transformCount == 0);
    assert(scene->spriteCount == 0);
    assert(scene->collisionCount == 0);
    assert(scene->rootObjectCount == 0);
    
    scene_destroy(scene);
    printf("✓ Scene creation/destruction test passed\n");
}

void test_scene_null_input_handling(void) {
    // Test null inputs for creation
    Scene* scene = scene_create(NULL, 100);
    assert(scene != NULL); // Should use default name
    assert(strcmp(scene->name, "UnnamedScene") == 0);
    scene_destroy(scene);
    
    // Test zero capacity
    scene = scene_create("ZeroTest", 0);
    assert(scene == NULL); // Should fail
    
    // Test null scene destruction (should not crash)
    scene_destroy(NULL);
    
    printf("✓ Scene null input handling test passed\n");
}

void test_scene_state_management(void) {
    Scene* scene = scene_create("StateTest", 10);
    
    // Test initial state
    assert(scene_get_state(scene) == SCENE_STATE_INACTIVE);
    
    // Test state transitions
    SceneResult result = scene_set_state(scene, SCENE_STATE_LOADING);
    assert(result == SCENE_OK);
    assert(scene_get_state(scene) == SCENE_STATE_LOADING);
    
    result = scene_set_state(scene, SCENE_STATE_ACTIVE);
    assert(result == SCENE_OK);
    assert(scene_get_state(scene) == SCENE_STATE_ACTIVE);
    assert(scene_is_active(scene));
    
    result = scene_set_state(scene, SCENE_STATE_PAUSED);
    assert(result == SCENE_OK);
    assert(scene_get_state(scene) == SCENE_STATE_PAUSED);
    assert(!scene_is_active(scene));
    
    result = scene_set_state(scene, SCENE_STATE_UNLOADING);
    assert(result == SCENE_OK);
    assert(scene_get_state(scene) == SCENE_STATE_UNLOADING);
    
    // Test null input
    result = scene_set_state(NULL, SCENE_STATE_ACTIVE);
    assert(result == SCENE_ERROR_NULL_POINTER);
    
    SceneState state = scene_get_state(NULL);
    assert(state == SCENE_STATE_INACTIVE);
    
    scene_destroy(scene);
    printf("✓ Scene state management test passed\n");
}

void test_scene_time_scale(void) {
    Scene* scene = scene_create("TimeTest", 10);
    
    // Test default time scale
    assert(scene_get_time_scale(scene) == 1.0f);
    
    // Test setting time scale
    scene_set_time_scale(scene, 2.5f);
    assert(scene_get_time_scale(scene) == 2.5f);
    
    scene_set_time_scale(scene, 0.0f);
    assert(scene_get_time_scale(scene) == 0.0f);
    
    // Test null input
    scene_set_time_scale(NULL, 1.0f); // Should not crash
    float timeScale = scene_get_time_scale(NULL);
    assert(timeScale == 1.0f); // Should return default
    
    scene_destroy(scene);
    printf("✓ Scene time scale test passed\n");
}

void test_scene_gameobject_management(void) {
    // Initialize component system for this test
    component_registry_init();
    transform_component_register();
    
    Scene* scene = scene_create("GameObjectTest", 10);
    
    // Test initial state
    assert(scene_get_game_object_count(scene) == 0);
    assert(scene_get_active_object_count(scene) == 0);
    
    // Create GameObjects
    GameObject* obj1 = game_object_create(scene);
    GameObject* obj2 = game_object_create(scene);
    
    assert(obj1 != NULL);
    assert(obj2 != NULL);
    assert(scene_get_game_object_count(scene) == 2);
    
    // Test finding by ID
    GameObject* found = scene_find_game_object_by_id(scene, game_object_get_id(obj1));
    assert(found == obj1);
    
    found = scene_find_game_object_by_id(scene, 999999);
    assert(found == NULL); // Non-existent ID
    
    // Test removal
    SceneResult result = scene_remove_game_object(scene, obj1);
    assert(result == SCENE_OK);
    assert(scene_get_game_object_count(scene) == 1);
    
    // Try to remove already removed object
    result = scene_remove_game_object(scene, obj1);
    assert(result == SCENE_ERROR_OBJECT_NOT_FOUND);
    
    // Test null inputs
    result = scene_add_game_object(NULL, obj2);
    assert(result == SCENE_ERROR_NULL_POINTER);
    
    result = scene_add_game_object(scene, NULL);
    assert(result == SCENE_ERROR_NULL_POINTER);
    
    result = scene_remove_game_object(NULL, obj2);
    assert(result == SCENE_ERROR_NULL_POINTER);
    
    found = scene_find_game_object_by_id(NULL, 1);
    assert(found == NULL);
    
    uint32_t count = scene_get_game_object_count(NULL);
    assert(count == 0);
    
    scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Scene GameObject management test passed\n");
}

void test_scene_component_systems(void) {
    component_registry_init();
    transform_component_register();
    
    Scene* scene = scene_create("SystemTest", 100);
    
    // Test system registration
    SceneResult result = scene_register_component_system(scene, 
                                                        COMPONENT_TYPE_TRANSFORM,
                                                        NULL, // No batch update function for this test
                                                        NULL, // No render function
                                                        0);   // Priority 0
    assert(result == SCENE_OK);
    
    // Test enabling/disabling systems
    result = scene_enable_component_system(scene, COMPONENT_TYPE_TRANSFORM, false);
    assert(result == SCENE_OK);
    
    result = scene_enable_component_system(scene, COMPONENT_TYPE_TRANSFORM, true);
    assert(result == SCENE_OK);
    
    // Test non-existent system
    result = scene_enable_component_system(scene, COMPONENT_TYPE_SPRITE, true);
    assert(result == SCENE_ERROR_SYSTEM_NOT_FOUND);
    
    // Test null inputs
    result = scene_register_component_system(NULL, COMPONENT_TYPE_TRANSFORM, NULL, NULL, 0);
    assert(result == SCENE_ERROR_NULL_POINTER);
    
    result = scene_enable_component_system(NULL, COMPONENT_TYPE_TRANSFORM, true);
    assert(result == SCENE_ERROR_NULL_POINTER);
    
    scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Scene component systems test passed\n");
}

void test_scene_updates(void) {
    component_registry_init();
    transform_component_register();
    
    Scene* scene = scene_create("UpdateTest", 100);
    register_default_systems(scene);
    
    // Create objects with components
    GameObject* obj1 = game_object_create(scene);
    GameObject* obj2 = game_object_create(scene);
    
    scene_set_state(scene, SCENE_STATE_ACTIVE);
    
    // Test update
    float deltaTime = 0.016f; // 60 FPS
    scene_update(scene, deltaTime);
    
    // After update, time and frame count should increase
    assert(scene->totalTime > 0.0f);
    assert(scene->frameCount == 1);
    
    // Test fixed update
    scene_fixed_update(scene, deltaTime);
    
    // Test render
    scene_render(scene);
    
    // Test update on inactive scene (should do nothing)
    scene_set_state(scene, SCENE_STATE_PAUSED);
    float oldTime = scene->totalTime;
    uint32_t oldFrameCount = scene->frameCount;
    
    scene_update(scene, deltaTime);
    assert(scene->totalTime == oldTime);
    assert(scene->frameCount == oldFrameCount);
    
    // Test null input
    scene_update(NULL, deltaTime); // Should not crash
    scene_fixed_update(NULL, deltaTime); // Should not crash
    scene_render(NULL); // Should not crash
    
    scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Scene updates test passed\n");
}

void test_scene_resource_access(void) {
    Scene* scene = scene_create("ResourceTest", 10);
    
    // Test pool access
    ObjectPool* pool = scene_get_gameobject_pool(scene);
    assert(pool != NULL);
    
    pool = scene_get_component_pool(scene, COMPONENT_TYPE_TRANSFORM);
    assert(pool != NULL);
    
    // Test null inputs
    pool = scene_get_gameobject_pool(NULL);
    assert(pool == NULL);
    
    pool = scene_get_component_pool(NULL, COMPONENT_TYPE_TRANSFORM);
    assert(pool == NULL);
    
    scene_destroy(scene);
    printf("✓ Scene resource access test passed\n");
}

void test_scene_debug_functions(void) {
    Scene* scene = scene_create("DebugTest", 10);
    
    // Test stats printing (should not crash)
    scene_print_stats(scene);
    scene_print_stats(NULL);
    
    // Test memory usage calculation
    uint32_t usage = scene_get_memory_usage(scene);
    assert(usage > 0); // Should have some memory usage
    
    usage = scene_get_memory_usage(NULL);
    assert(usage == 0); // Null should return 0
    
    scene_destroy(scene);
    printf("✓ Scene debug functions test passed\n");
}

void test_scene_capacity_limits(void) {
    Scene* scene = scene_create("CapacityTest", 2); // Very small capacity
    component_registry_init();
    transform_component_register();
    
    // Fill scene to capacity
    GameObject* obj1 = game_object_create(scene);
    GameObject* obj2 = game_object_create(scene);
    
    assert(scene_get_game_object_count(scene) == 2);
    
    // Try to add beyond capacity
    GameObject* obj3 = game_object_create(scene);
    
    // The scene should either reject the object or handle gracefully
    // This test verifies the system doesn't crash when at capacity
    
    scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Scene capacity limits test passed\n");
}

// Global callback test state
static bool g_loadCalled = false;
static bool g_activateCalled = false;
static bool g_deactivateCalled = false;
static bool g_unloadCalled = false;

static void test_on_load(Scene* scene) {
    g_loadCalled = true;
}

static void test_on_activate(Scene* scene) {
    g_activateCalled = true;
}

static void test_on_deactivate(Scene* scene) {
    g_deactivateCalled = true;
}

static void test_on_unload(Scene* scene) {
    g_unloadCalled = true;
}

void test_scene_lifecycle_callbacks(void) {
    Scene* scene = scene_create("CallbackTest", 10);
    
    // Reset callback flags
    g_loadCalled = false;
    g_activateCalled = false;
    g_deactivateCalled = false;
    g_unloadCalled = false;
    
    // Set up callbacks
    scene->onLoad = test_on_load;
    scene->onActivate = test_on_activate;
    scene->onDeactivate = test_on_deactivate;
    scene->onUnload = test_on_unload;
    
    // Test state transitions trigger callbacks
    scene_set_state(scene, SCENE_STATE_LOADING);
    assert(g_loadCalled);
    
    scene_set_state(scene, SCENE_STATE_ACTIVE);
    assert(g_activateCalled);
    
    scene_set_state(scene, SCENE_STATE_PAUSED);
    assert(g_deactivateCalled);
    
    scene_set_state(scene, SCENE_STATE_UNLOADING);
    assert(g_unloadCalled);
    
    scene_destroy(scene);
    printf("✓ Scene lifecycle callbacks test passed\n");
}

int run_scene_tests(void) {
    printf("Running scene tests...\n");
    
    test_scene_creation_destruction();
    test_scene_null_input_handling();
    test_scene_state_management();
    test_scene_time_scale();
    test_scene_gameobject_management();
    test_scene_component_systems();
    test_scene_updates();
    test_scene_resource_access();
    test_scene_debug_functions();
    test_scene_capacity_limits();
    test_scene_lifecycle_callbacks();
    
    printf("All scene tests passed! ✓\n\n");
    return 0;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_scene_tests();
}
#endif