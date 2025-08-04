#include "../../src/components/component_factory.h"
#include "../../src/components/transform_component.h"
#include <assert.h>
#include <stdio.h>

// Mock GameObject for testing
typedef struct MockGameObject {
    uint32_t id;
    const char* name;
} MockGameObject;

void test_component_factory_initialization(void) {
    ComponentResult result = component_factory_init();
    assert(result == COMPONENT_OK);
    
    // Should initialize with no registered types initially
    uint32_t typeCount = component_factory_get_registered_type_count();
    assert(typeCount == 0);
    
    component_factory_shutdown();
    printf("✓ Component factory initialization test passed\n");
}

void test_component_factory_type_registration(void) {
    ComponentResult result = component_factory_init();
    assert(result == COMPONENT_OK);
    
    // Register all standard component types
    result = component_factory_register_all_types();
    assert(result == COMPONENT_OK);
    
    // Should have registered at least the basic types
    uint32_t typeCount = component_factory_get_registered_type_count();
    assert(typeCount > 0);
    assert(typeCount <= 8); // We have 8 basic component types defined
    
    component_factory_shutdown();
    printf("✓ Component factory type registration test passed\n");
}

void test_component_factory_generic_creation(void) {
    component_factory_init();
    component_factory_register_all_types();
    
    MockGameObject gameObject = {1, "FactoryTest"};
    
    // Create component through factory
    Component* component = component_factory_create(COMPONENT_TYPE_TRANSFORM, 
                                                   (GameObject*)&gameObject);
    assert(component != NULL);
    assert(component->type == COMPONENT_TYPE_TRANSFORM);
    assert(component->gameObject == (GameObject*)&gameObject);
    assert(component->enabled == true);
    
    // Destroy component through factory
    ComponentResult result = component_factory_destroy(component);
    assert(result == COMPONENT_OK);
    
    // Test invalid type
    Component* invalidComponent = component_factory_create(COMPONENT_TYPE_NONE, 
                                                         (GameObject*)&gameObject);
    assert(invalidComponent == NULL);
    
    // Test null gameObject
    Component* nullComponent = component_factory_create(COMPONENT_TYPE_TRANSFORM, NULL);
    assert(nullComponent == NULL);
    
    component_factory_shutdown();
    printf("✓ Component factory generic creation test passed\n");
}

void test_component_factory_typed_creation(void) {
    component_factory_init();
    component_factory_register_all_types();
    
    MockGameObject gameObject = {1, "TypedTest"};
    
    // Create transform component through typed factory
    TransformComponent* transform = component_factory_create_transform((GameObject*)&gameObject);
    assert(transform != NULL);
    assert(transform->base.type == COMPONENT_TYPE_TRANSFORM);
    assert(transform->base.gameObject == (GameObject*)&gameObject);
    
    // Verify it's a proper transform component
    assert(transform->x == 0.0f);
    assert(transform->y == 0.0f);
    assert(transform->rotation == 0.0f);
    assert(transform->matrixDirty == true);
    
    // Destroy through factory
    ComponentResult result = component_factory_destroy((Component*)transform);
    assert(result == COMPONENT_OK);
    
    // Test null gameObject
    TransformComponent* nullTransform = component_factory_create_transform(NULL);
    assert(nullTransform == NULL);
    
    component_factory_shutdown();
    printf("✓ Component factory typed creation test passed\n");
}

void test_component_factory_pool_validation(void) {
    component_factory_init();
    component_factory_register_all_types();
    
    // Validate all pools are properly set up
    ComponentResult result = component_factory_validate_all_pools();
    assert(result == COMPONENT_OK);
    
    component_factory_shutdown();
    printf("✓ Component factory pool validation test passed\n");
}

void test_component_factory_stats(void) {
    component_factory_init();
    component_factory_register_all_types();
    
    // This should not crash and should print meaningful stats
    component_factory_print_stats();
    
    component_factory_shutdown();
    printf("✓ Component factory stats test passed\n");
}

void test_component_factory_batch_operations(void) {
    component_factory_init();
    component_factory_register_all_types();
    
    MockGameObject gameObject = {1, "BatchTest"};
    
    // Create multiple transform components (only type implemented)
    Component* transform1 = component_factory_create(COMPONENT_TYPE_TRANSFORM, (GameObject*)&gameObject);
    Component* transform2 = component_factory_create(COMPONENT_TYPE_TRANSFORM, (GameObject*)&gameObject);
    Component* transform3 = component_factory_create(COMPONENT_TYPE_TRANSFORM, (GameObject*)&gameObject);
    
    assert(transform1 != NULL);
    assert(transform2 != NULL);
    assert(transform3 != NULL);
    
    // All should be transform type with same gameObject
    assert(transform1->type == COMPONENT_TYPE_TRANSFORM);
    assert(transform2->type == COMPONENT_TYPE_TRANSFORM);
    assert(transform3->type == COMPONENT_TYPE_TRANSFORM);
    
    assert(transform1->gameObject == (GameObject*)&gameObject);
    assert(transform2->gameObject == (GameObject*)&gameObject);
    assert(transform3->gameObject == (GameObject*)&gameObject);
    
    // Should have different IDs
    assert(transform1->id != transform2->id);
    assert(transform2->id != transform3->id);
    assert(transform1->id != transform3->id);
    
    // Destroy all components
    assert(component_factory_destroy(transform1) == COMPONENT_OK);
    assert(component_factory_destroy(transform2) == COMPONENT_OK);
    assert(component_factory_destroy(transform3) == COMPONENT_OK);
    
    component_factory_shutdown();
    printf("✓ Component factory batch operations test passed\n");
}

void test_component_factory_error_handling(void) {
    component_factory_init();
    
    // Try to validate pools without any registration first
    ComponentResult result = component_factory_validate_all_pools();
    assert(result != COMPONENT_OK);
    
    MockGameObject gameObject = {1, "ErrorTest"};
    
    // Try to create component without registering types first
    Component* component = component_factory_create(COMPONENT_TYPE_TRANSFORM, (GameObject*)&gameObject);
    // Note: Transform auto-registers, so this may succeed
    if (component) {
        component_factory_destroy(component);
    }
    
    // Try to destroy null component
    result = component_factory_destroy(NULL);
    assert(result == COMPONENT_ERROR_NULL_POINTER);
    
    // After auto-registration, validation should now succeed
    result = component_factory_validate_all_pools();
    assert(result == COMPONENT_OK);
    
    component_factory_shutdown();
    printf("✓ Component factory error handling test passed\n");
}

void test_component_factory_multiple_init_shutdown(void) {
    // Test multiple init/shutdown cycles
    for (int i = 0; i < 3; i++) {
        assert(component_factory_init() == COMPONENT_OK);
        assert(component_factory_register_all_types() == COMPONENT_OK);
        assert(component_factory_get_registered_type_count() > 0);
        component_factory_shutdown();
    }
    
    printf("✓ Component factory multiple init/shutdown test passed\n");
}

void test_component_factory_thread_safety_assumptions(void) {
    // Note: This engine assumes single-threaded usage, but we test
    // that the factory doesn't maintain global state that would be
    // problematic for future multi-threading
    
    component_factory_init();
    component_factory_register_all_types();
    
    MockGameObject gameObject1 = {1, "Thread1"};
    MockGameObject gameObject2 = {2, "Thread2"};
    
    // Create components for different game objects
    Component* comp1 = component_factory_create(COMPONENT_TYPE_TRANSFORM, (GameObject*)&gameObject1);
    Component* comp2 = component_factory_create(COMPONENT_TYPE_TRANSFORM, (GameObject*)&gameObject2);
    
    assert(comp1 != NULL && comp2 != NULL);
    assert(comp1 != comp2);
    assert(comp1->gameObject != comp2->gameObject);
    assert(comp1->id != comp2->id); // Should have unique IDs
    
    component_factory_destroy(comp1);
    component_factory_destroy(comp2);
    component_factory_shutdown();
    
    printf("✓ Component factory thread safety assumptions test passed\n");
}

// Test runner function
int run_component_factory_tests(void) {
    printf("=== Component Factory Tests ===\n");
    
    int failures = 0;
    
    test_component_factory_initialization();
    test_component_factory_type_registration();
    test_component_factory_generic_creation();
    test_component_factory_typed_creation();
    test_component_factory_pool_validation();
    test_component_factory_stats();
    test_component_factory_batch_operations();
    test_component_factory_error_handling();
    test_component_factory_multiple_init_shutdown();
    test_component_factory_thread_safety_assumptions();
    
    printf("Component factory tests completed with %d failures\n", failures);
    return failures;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_component_factory_tests();
}
#endif