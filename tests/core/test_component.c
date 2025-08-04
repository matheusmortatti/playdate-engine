#include "../../src/core/component.h"
#include "../../src/core/component_registry.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Mock GameObject for testing
typedef struct MockGameObject {
    uint32_t id;
    const char* name;
} MockGameObject;

// Mock component vtable functions
static bool mock_init_called = false;
static bool mock_destroy_called = false;
static bool mock_update_called = false;
static bool mock_enabled_called = false;
static bool mock_disabled_called = false;

static void mock_component_init(Component* component, GameObject* gameObject) {
    (void)component;
    (void)gameObject;
    mock_init_called = true;
}

static void mock_component_destroy(Component* component) {
    (void)component;
    mock_destroy_called = true;
}

static void mock_component_update(Component* component, float deltaTime) {
    (void)component;
    (void)deltaTime;
    mock_update_called = true;
}

static void mock_component_on_enabled(Component* component) {
    (void)component;
    mock_enabled_called = true;
}

static void mock_component_on_disabled(Component* component) {
    (void)component;
    mock_disabled_called = true;
}

static const ComponentVTable mockVTable = {
    .init = mock_component_init,
    .destroy = mock_component_destroy,
    .update = mock_component_update,
    .fixedUpdate = NULL,
    .render = NULL,
    .onEnabled = mock_component_on_enabled,
    .onDisabled = mock_component_on_disabled,
    .onGameObjectDestroyed = NULL,
    .getSerializedSize = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

void reset_mock_flags(void) {
    mock_init_called = false;
    mock_destroy_called = false;
    mock_update_called = false;
    mock_enabled_called = false;
    mock_disabled_called = false;
}

void test_component_initialization(void) {
    MockGameObject gameObject = {1, "TestObject"};
    Component component;
    
    ComponentResult result = component_init(&component, COMPONENT_TYPE_SPRITE, 
                                          &mockVTable, (GameObject*)&gameObject);
    
    assert(result == COMPONENT_OK);
    assert(component.type == COMPONENT_TYPE_SPRITE);
    assert(component.vtable == &mockVTable);
    assert(component.gameObject == (GameObject*)&gameObject);
    assert(component.enabled == true);
    assert(component.id == 0); // ID set by registry, not component_init
    
    printf("✓ Component initialization test passed\n");
}

void test_component_null_pointer_validation(void) {
    MockGameObject gameObject = {1, "TestObject"};
    Component component;
    
    // Null component
    ComponentResult result = component_init(NULL, COMPONENT_TYPE_SPRITE, 
                                          &mockVTable, (GameObject*)&gameObject);
    assert(result == COMPONENT_ERROR_NULL_POINTER);
    
    // Null vtable
    result = component_init(&component, COMPONENT_TYPE_SPRITE, 
                           NULL, (GameObject*)&gameObject);
    assert(result == COMPONENT_ERROR_NULL_POINTER);
    
    // Null gameObject
    result = component_init(&component, COMPONENT_TYPE_SPRITE, 
                           &mockVTable, NULL);
    assert(result == COMPONENT_ERROR_NULL_POINTER);
    
    // Invalid type
    result = component_init(&component, COMPONENT_TYPE_NONE, 
                           &mockVTable, (GameObject*)&gameObject);
    assert(result == COMPONENT_ERROR_INVALID_TYPE);
    
    printf("✓ Component null pointer validation test passed\n");
}

void test_component_type_checking(void) {
    MockGameObject gameObject = {1, "TestObject"};
    Component component;
    
    ComponentResult result = component_init(&component, COMPONENT_TYPE_SPRITE, 
                                          &mockVTable, (GameObject*)&gameObject);
    assert(result == COMPONENT_OK);
    
    // Test single type checking
    assert(component_is_type(&component, COMPONENT_TYPE_SPRITE));
    assert(!component_is_type(&component, COMPONENT_TYPE_TRANSFORM));
    assert(!component_is_type(&component, COMPONENT_TYPE_COLLISION));
    
    // Test combined types
    component.type = COMPONENT_TYPE_SPRITE | COMPONENT_TYPE_COLLISION;
    assert(component_is_type(&component, COMPONENT_TYPE_SPRITE));
    assert(component_is_type(&component, COMPONENT_TYPE_COLLISION));
    assert(!component_is_type(&component, COMPONENT_TYPE_TRANSFORM));
    
    // Test null component
    assert(!component_is_type(NULL, COMPONENT_TYPE_SPRITE));
    
    printf("✓ Component type checking test passed\n");
}

void test_component_enable_disable(void) {
    MockGameObject gameObject = {1, "TestObject"};
    Component component;
    
    ComponentResult result = component_init(&component, COMPONENT_TYPE_SPRITE, 
                                          &mockVTable, (GameObject*)&gameObject);
    assert(result == COMPONENT_OK);
    
    // Initial state should be enabled
    assert(component_is_enabled(&component));
    
    reset_mock_flags();
    
    // Disable component
    component_set_enabled(&component, false);
    assert(!component_is_enabled(&component));
    assert(mock_disabled_called);
    
    reset_mock_flags();
    
    // Enable component
    component_set_enabled(&component, true);
    assert(component_is_enabled(&component));
    assert(mock_enabled_called);
    
    // Test null component
    assert(!component_is_enabled(NULL));
    
    printf("✓ Component enable/disable test passed\n");
}

void test_component_virtual_function_calls(void) {
    MockGameObject gameObject = {1, "TestObject"};
    Component component;
    
    ComponentResult result = component_init(&component, COMPONENT_TYPE_SPRITE, 
                                          &mockVTable, (GameObject*)&gameObject);
    assert(result == COMPONENT_OK);
    
    reset_mock_flags();
    
    // Test update call
    component_call_update(&component, 0.016f);
    assert(mock_update_called);
    
    reset_mock_flags();
    
    // Test vtable calls when disabled (should not call)
    component_set_enabled(&component, false);
    component_call_update(&component, 0.016f);
    assert(!mock_update_called); // Should not be called when disabled
    
    // Test null component (should not crash)
    component_call_update(NULL, 0.016f);
    component_call_render(NULL);
    component_call_on_enabled(NULL);
    component_call_on_disabled(NULL);
    
    printf("✓ Component virtual function calls test passed\n");
}

void test_component_type_to_string(void) {
    assert(strcmp(component_type_to_string(COMPONENT_TYPE_TRANSFORM), "Transform") == 0);
    assert(strcmp(component_type_to_string(COMPONENT_TYPE_SPRITE), "Sprite") == 0);
    assert(strcmp(component_type_to_string(COMPONENT_TYPE_COLLISION), "Collision") == 0);
    assert(strcmp(component_type_to_string(COMPONENT_TYPE_SCRIPT), "Script") == 0);
    assert(strcmp(component_type_to_string(COMPONENT_TYPE_AUDIO), "Audio") == 0);
    assert(strcmp(component_type_to_string(COMPONENT_TYPE_ANIMATION), "Animation") == 0);
    assert(strcmp(component_type_to_string(COMPONENT_TYPE_PARTICLES), "Particles") == 0);
    assert(strcmp(component_type_to_string(COMPONENT_TYPE_UI), "UI") == 0);
    assert(strcmp(component_type_to_string(999), "Unknown") == 0);
    
    printf("✓ Component type to string test passed\n");
}

void test_component_structure_alignment(void) {
    // Verify the Component structure is properly aligned
    assert(sizeof(Component) >= 32); // Minimum 32 bytes for cache alignment
    assert(sizeof(Component) % 16 == 0); // Must be 16-byte aligned for ARM Cortex-M7
    assert(sizeof(Component) == 48); // Should be exactly 48 bytes
    
    // Verify field offsets and sizes
    Component component;
    assert(sizeof(component.type) == 4);
    assert(sizeof(component.vtable) == 8);
    assert(sizeof(component.gameObject) == 8);
    assert(sizeof(component.id) == 4);
    assert(sizeof(component.enabled) == 1);
    
    printf("✓ Component structure alignment test passed\n");
}

// Test runner function
int run_component_tests(void) {
    printf("=== Component System Tests ===\n");
    
    int failures = 0;
    
    // Run all tests and catch assertion failures
    test_component_initialization();
    test_component_null_pointer_validation();
    test_component_type_checking();
    test_component_enable_disable();
    test_component_virtual_function_calls();
    test_component_type_to_string();
    test_component_structure_alignment();
    
    printf("Component tests completed with %d failures\n", failures);
    return failures;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_component_tests();
}
#endif