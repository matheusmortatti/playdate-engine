#include "../../src/core/component_registry.h"
#include "../../src/components/transform_component.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

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

void test_component_registry_initialization(void) {
    ComponentResult result = component_registry_init();
    assert(result == COMPONENT_OK);
    
    // Should start with no registered types
    assert(!component_registry_is_type_registered(COMPONENT_TYPE_TRANSFORM));
    assert(!component_registry_is_type_registered(COMPONENT_TYPE_SPRITE));
    
    component_registry_shutdown();
    printf("✓ Component registry initialization test passed\n");
}

void test_component_type_registration(void) {
    component_registry_init();
    
    ComponentResult result = component_registry_register_type(
        COMPONENT_TYPE_TRANSFORM,
        sizeof(TransformComponent),
        100,
        &mockVTable,
        "Transform"
    );
    
    assert(result == COMPONENT_OK);
    assert(component_registry_is_type_registered(COMPONENT_TYPE_TRANSFORM));
    
    // Get type info and verify
    const ComponentTypeInfo* info = component_registry_get_type_info(COMPONENT_TYPE_TRANSFORM);
    assert(info != NULL);
    assert(info->type == COMPONENT_TYPE_TRANSFORM);
    assert(info->componentSize >= sizeof(TransformComponent));
    assert(info->poolCapacity == 100);
    assert(info->defaultVTable == &mockVTable);
    assert(strcmp(info->typeName, "Transform") == 0);
    assert(info->registered == true);
    
    // Test duplicate registration
    result = component_registry_register_type(
        COMPONENT_TYPE_TRANSFORM,
        sizeof(TransformComponent),
        100,
        &mockVTable,
        "Transform"
    );
    
    assert(result == COMPONENT_ERROR_ALREADY_EXISTS);
    
    component_registry_shutdown();
    printf("✓ Component type registration test passed\n");
}

void test_component_registration_validation(void) {
    component_registry_init();
    
    // Test null vtable
    ComponentResult result = component_registry_register_type(
        COMPONENT_TYPE_TRANSFORM,
        sizeof(TransformComponent),
        100,
        NULL,
        "Transform"
    );
    assert(result == COMPONENT_ERROR_NULL_POINTER);
    
    // Test null type name
    result = component_registry_register_type(
        COMPONENT_TYPE_TRANSFORM,
        sizeof(TransformComponent),
        100,
        &mockVTable,
        NULL
    );
    assert(result == COMPONENT_ERROR_NULL_POINTER);
    
    // Test invalid type (not power of 2)
    result = component_registry_register_type(
        (ComponentType)3, // Not a power of 2
        sizeof(TransformComponent),
        100,
        &mockVTable,
        "Invalid"
    );
    assert(result == COMPONENT_ERROR_INVALID_TYPE);
    
    component_registry_shutdown();
    printf("✓ Component registration validation test passed\n");
}

void test_component_creation_destruction(void) {
    component_registry_init();
    
    component_registry_register_type(
        COMPONENT_TYPE_TRANSFORM,
        sizeof(TransformComponent),
        10,
        &mockVTable,
        "Transform"
    );
    
    MockGameObject gameObject = {1, "TestObject"};
    
    // Create component
    Component* component = component_registry_create(COMPONENT_TYPE_TRANSFORM, 
                                                   (GameObject*)&gameObject);
    assert(component != NULL);
    assert(component->type == COMPONENT_TYPE_TRANSFORM);
    assert(component->gameObject == (GameObject*)&gameObject);
    assert(component->enabled == true);
    assert(component->vtable == &mockVTable);
    assert(component->id > 0); // Should be assigned by registry
    
    // Verify component count
    uint32_t count = component_registry_get_component_count(COMPONENT_TYPE_TRANSFORM);
    assert(count == 1);
    
    // Create multiple components to test pool
    Component* components[10];
    for (int i = 0; i < 9; i++) { // We already created one
        components[i] = component_registry_create(COMPONENT_TYPE_TRANSFORM, 
                                                (GameObject*)&gameObject);
        assert(components[i] != NULL);
    }
    
    // Pool should be full now (10 components total)
    Component* shouldBeNull = component_registry_create(COMPONENT_TYPE_TRANSFORM, 
                                                      (GameObject*)&gameObject);
    assert(shouldBeNull == NULL);
    
    // Destroy a component
    ComponentResult result = component_registry_destroy(component);
    assert(result == COMPONENT_OK);
    
    // Should be able to create one more now
    Component* newComponent = component_registry_create(COMPONENT_TYPE_TRANSFORM, 
                                                      (GameObject*)&gameObject);
    assert(newComponent != NULL);
    
    component_registry_shutdown();
    printf("✓ Component creation/destruction test passed\n");
}

void test_component_creation_validation(void) {
    component_registry_init();
    
    component_registry_register_type(
        COMPONENT_TYPE_TRANSFORM,
        sizeof(TransformComponent),
        10,
        &mockVTable,
        "Transform"
    );
    
    MockGameObject gameObject = {1, "TestObject"};
    
    // Test null gameObject
    Component* component = component_registry_create(COMPONENT_TYPE_TRANSFORM, NULL);
    assert(component == NULL);
    
    // Test unregistered type
    component = component_registry_create(COMPONENT_TYPE_SPRITE, (GameObject*)&gameObject);
    assert(component == NULL);
    
    // Test invalid type
    component = component_registry_create(COMPONENT_TYPE_NONE, (GameObject*)&gameObject);
    assert(component == NULL);
    
    component_registry_shutdown();
    printf("✓ Component creation validation test passed\n");
}

void test_component_registry_queries(void) {
    component_registry_init();
    
    // Register multiple types
    component_registry_register_type(COMPONENT_TYPE_TRANSFORM, sizeof(TransformComponent), 100, &mockVTable, "Transform");
    component_registry_register_type(COMPONENT_TYPE_SPRITE, sizeof(Component), 50, &mockVTable, "Sprite");
    
    MockGameObject gameObject = {1, "TestObject"};
    
    // Create some components
    Component* transform1 = component_registry_create(COMPONENT_TYPE_TRANSFORM, (GameObject*)&gameObject);
    Component* transform2 = component_registry_create(COMPONENT_TYPE_TRANSFORM, (GameObject*)&gameObject);
    Component* sprite1 = component_registry_create(COMPONENT_TYPE_SPRITE, (GameObject*)&gameObject);
    
    assert(transform1 != NULL && transform2 != NULL && sprite1 != NULL);
    
    // Test component counts
    assert(component_registry_get_component_count(COMPONENT_TYPE_TRANSFORM) == 2);
    assert(component_registry_get_component_count(COMPONENT_TYPE_SPRITE) == 1);
    assert(component_registry_get_component_count(COMPONENT_TYPE_COLLISION) == 0);
    
    // Test pool access
    ObjectPool* transformPool = component_registry_get_pool(COMPONENT_TYPE_TRANSFORM);
    ObjectPool* spritePool = component_registry_get_pool(COMPONENT_TYPE_SPRITE);
    ObjectPool* nullPool = component_registry_get_pool(COMPONENT_TYPE_COLLISION);
    
    assert(transformPool != NULL);
    assert(spritePool != NULL);
    assert(nullPool == NULL);
    
    // Test memory usage
    uint32_t memoryUsage = component_registry_get_total_memory_usage();
    assert(memoryUsage > 0);
    
    component_registry_shutdown();
    printf("✓ Component registry queries test passed\n");
}

void test_component_registry_stats(void) {
    component_registry_init();
    
    component_registry_register_type(COMPONENT_TYPE_TRANSFORM, sizeof(TransformComponent), 10, &mockVTable, "Transform");
    
    // This should not crash
    component_registry_print_stats();
    
    component_registry_shutdown();
    printf("✓ Component registry stats test passed\n");
}

void test_multiple_type_registration(void) {
    component_registry_init();
    
    // Register all basic component types
    ComponentResult results[8];
    results[0] = component_registry_register_type(COMPONENT_TYPE_TRANSFORM, 64, 100, &mockVTable, "Transform");
    results[1] = component_registry_register_type(COMPONENT_TYPE_SPRITE, 48, 200, &mockVTable, "Sprite");
    results[2] = component_registry_register_type(COMPONENT_TYPE_COLLISION, 32, 150, &mockVTable, "Collision");
    results[3] = component_registry_register_type(COMPONENT_TYPE_SCRIPT, 40, 50, &mockVTable, "Script");
    results[4] = component_registry_register_type(COMPONENT_TYPE_AUDIO, 36, 25, &mockVTable, "Audio");
    results[5] = component_registry_register_type(COMPONENT_TYPE_ANIMATION, 56, 75, &mockVTable, "Animation");
    results[6] = component_registry_register_type(COMPONENT_TYPE_PARTICLES, 72, 30, &mockVTable, "Particles");
    results[7] = component_registry_register_type(COMPONENT_TYPE_UI, 44, 80, &mockVTable, "UI");
    
    // All registrations should succeed
    for (int i = 0; i < 8; i++) {
        assert(results[i] == COMPONENT_OK);
    }
    
    // All types should be registered
    assert(component_registry_is_type_registered(COMPONENT_TYPE_TRANSFORM));
    assert(component_registry_is_type_registered(COMPONENT_TYPE_SPRITE));
    assert(component_registry_is_type_registered(COMPONENT_TYPE_COLLISION));
    assert(component_registry_is_type_registered(COMPONENT_TYPE_SCRIPT));
    assert(component_registry_is_type_registered(COMPONENT_TYPE_AUDIO));
    assert(component_registry_is_type_registered(COMPONENT_TYPE_ANIMATION));
    assert(component_registry_is_type_registered(COMPONENT_TYPE_PARTICLES));
    assert(component_registry_is_type_registered(COMPONENT_TYPE_UI));
    
    component_registry_shutdown();
    printf("✓ Multiple type registration test passed\n");
}

// Test runner function
int run_component_registry_tests(void) {
    printf("=== Component Registry Tests ===\n");
    
    int failures = 0;
    
    test_component_registry_initialization();
    test_component_type_registration();
    test_component_registration_validation();
    test_component_creation_destruction();
    test_component_creation_validation();
    test_component_registry_queries();
    test_component_registry_stats();
    test_multiple_type_registration();
    
    printf("Component registry tests completed with %d failures\n", failures);
    return failures;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_component_registry_tests();
}
#endif