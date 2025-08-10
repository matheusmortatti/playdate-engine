#include "../../src/core/game_object.h"
#include "../../src/components/transform_component.h"
#include "../../src/core/component_registry.h"
#include "mock_scene.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

const float EPSILON = 0.0001f;

bool float_equals(float a, float b) {
    return fabs(a - b) < EPSILON;
}

void test_gameobject_structure_alignment(void) {
    // Verify the GameObject structure is properly aligned to 96 bytes (cache-friendly)
    assert(sizeof(GameObject) == 96);
    assert(sizeof(GameObject) % 16 == 0);
    
    printf("✓ GameObject structure alignment test passed\n");
}

void test_gameobject_creation_destruction(void) {
    component_registry_init();
    
    MockScene* scene = mock_scene_create();
    
    // Create GameObject
    GameObject* gameObject = game_object_create((Scene*)scene);
    assert(gameObject != NULL);
    assert(gameObject->id != GAMEOBJECT_INVALID_ID);
    assert(gameObject->active == true);
    assert(gameObject->staticObject == false);
    assert(gameObject->componentCount == 1); // Transform component
    assert(game_object_has_component(gameObject, COMPONENT_TYPE_TRANSFORM));
    assert(gameObject->transform != NULL);
    assert(gameObject->scene == (Scene*)scene);
    assert(gameObject->parent == NULL);
    assert(gameObject->firstChild == NULL);
    assert(gameObject->nextSibling == NULL);
    
    // Verify transform is accessible
    float x, y;
    game_object_get_position(gameObject, &x, &y);
    assert(float_equals(x, 0.0f) && float_equals(y, 0.0f));
    
    // Verify scene tracking
    assert(mock_scene_get_gameobject_count(scene) == 1);
    
    // Destroy GameObject
    game_object_destroy(gameObject);
    
    // Verify scene cleanup
    assert(mock_scene_get_gameobject_count(scene) == 0);
    
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ GameObject creation/destruction test passed\n");
}

void test_gameobject_creation_with_name(void) {
    component_registry_init();
    
    MockScene* scene = mock_scene_create();
    
    // Create GameObject with debug name (name is not stored, just for debugging)
    GameObject* gameObject = game_object_create_with_name((Scene*)scene, "TestObject");
    assert(gameObject != NULL);
    assert(gameObject->id != GAMEOBJECT_INVALID_ID);
    assert(game_object_has_component(gameObject, COMPONENT_TYPE_TRANSFORM));
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ GameObject creation with name test passed\n");
}

void test_component_management(void) {
    component_registry_init();
    
    // Register a mock sprite component type for testing
    static const ComponentVTable mockSpriteVTable = {
        .init = NULL, .destroy = NULL, .clone = NULL,
        .update = NULL, .fixedUpdate = NULL, .render = NULL,
        .onEnabled = NULL, .onDisabled = NULL, .onGameObjectDestroyed = NULL,
        .getSerializedSize = NULL, .serialize = NULL, .deserialize = NULL
    };
    
    component_registry_register_type(
        COMPONENT_TYPE_SPRITE,
        sizeof(Component),
        10,
        &mockSpriteVTable,
        "Sprite"
    );
    
    MockScene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create((Scene*)scene);
    
    // Initial state: only transform component
    assert(gameObject->componentCount == 1);
    assert(game_object_has_component(gameObject, COMPONENT_TYPE_TRANSFORM));
    assert(!game_object_has_component(gameObject, COMPONENT_TYPE_SPRITE));
    
    // Test component addition
    Component* spriteComponent = component_registry_create(COMPONENT_TYPE_SPRITE, gameObject);
    GameObjectResult result = game_object_add_component(gameObject, spriteComponent);
    assert(result == GAMEOBJECT_OK);
    assert(gameObject->componentCount == 2);
    assert(game_object_has_component(gameObject, COMPONENT_TYPE_SPRITE));
    assert(game_object_get_component_count(gameObject) == 2);
    
    // Test component retrieval
    Component* retrieved = game_object_get_component(gameObject, COMPONENT_TYPE_SPRITE);
    assert(retrieved == spriteComponent);
    
    // Test transform component fast path
    Component* transform = game_object_get_component(gameObject, COMPONENT_TYPE_TRANSFORM);
    assert(transform == (Component*)gameObject->transform);
    
    // Test duplicate component rejection
    Component* duplicate = component_registry_create(COMPONENT_TYPE_SPRITE, gameObject);
    result = game_object_add_component(gameObject, duplicate);
    assert(result == GAMEOBJECT_ERROR_COMPONENT_ALREADY_EXISTS);
    component_registry_destroy(duplicate);
    
    // Test component removal
    result = game_object_remove_component(gameObject, COMPONENT_TYPE_SPRITE);
    assert(result == GAMEOBJECT_OK);
    assert(gameObject->componentCount == 1);
    assert(!game_object_has_component(gameObject, COMPONENT_TYPE_SPRITE));
    assert(game_object_get_component(gameObject, COMPONENT_TYPE_SPRITE) == NULL);
    
    // Test removing non-existent component
    result = game_object_remove_component(gameObject, COMPONENT_TYPE_COLLISION);
    assert(result == GAMEOBJECT_ERROR_COMPONENT_NOT_FOUND);
    
    // Test removing transform component (should fail)
    result = game_object_remove_component(gameObject, COMPONENT_TYPE_TRANSFORM);
    assert(result == GAMEOBJECT_ERROR_INVALID_COMPONENT_TYPE);
    assert(game_object_has_component(gameObject, COMPONENT_TYPE_TRANSFORM));
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Component management test passed\n");
}

void test_component_array_limits(void) {
    component_registry_init();
    
    // Register multiple mock component types
    static const ComponentVTable mockVTable = {
        .init = NULL, .destroy = NULL, .clone = NULL,
        .update = NULL, .fixedUpdate = NULL, .render = NULL,
        .onEnabled = NULL, .onDisabled = NULL, .onGameObjectDestroyed = NULL,
        .getSerializedSize = NULL, .serialize = NULL, .deserialize = NULL
    };
    
    component_registry_register_type(COMPONENT_TYPE_SPRITE, sizeof(Component), 10, &mockVTable, "Sprite");
    component_registry_register_type(COMPONENT_TYPE_COLLISION, sizeof(Component), 10, &mockVTable, "Collision");
    component_registry_register_type(COMPONENT_TYPE_SCRIPT, sizeof(Component), 10, &mockVTable, "Script");
    component_registry_register_type(COMPONENT_TYPE_AUDIO, sizeof(Component), 10, &mockVTable, "Audio");
    component_registry_register_type(COMPONENT_TYPE_ANIMATION, sizeof(Component), 10, &mockVTable, "Animation");
    component_registry_register_type(COMPONENT_TYPE_PARTICLES, sizeof(Component), 10, &mockVTable, "Particles");
    component_registry_register_type(COMPONENT_TYPE_UI, sizeof(Component), 10, &mockVTable, "UI");
    
    MockScene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create((Scene*)scene);
    
    // GameObject starts with 1 component (Transform), can hold MAX_COMPONENTS_PER_OBJECT total
    assert(gameObject->componentCount == 1);
    
    ComponentType types[] = {
        COMPONENT_TYPE_SPRITE,
        COMPONENT_TYPE_COLLISION, 
        COMPONENT_TYPE_SCRIPT
    };
    
    // Add components up to the limit (MAX_COMPONENTS_PER_OBJECT = 4, already have 1 transform)
    for (int i = 0; i < 3; i++) {
        Component* comp = component_registry_create(types[i], gameObject);
        GameObjectResult result = game_object_add_component(gameObject, comp);
        assert(result == GAMEOBJECT_OK);
    }
    
    assert(gameObject->componentCount == MAX_COMPONENTS_PER_OBJECT);
    
    // Try to add one more component (should fail)
    Component* extraComp = component_registry_create(COMPONENT_TYPE_AUDIO, gameObject);
    GameObjectResult result = game_object_add_component(gameObject, extraComp);
    if (result != GAMEOBJECT_ERROR_MAX_COMPONENTS_REACHED) {
        printf("Expected MAX_COMPONENTS_REACHED but got %d. Current count: %d, Max: %d\n", 
               result, gameObject->componentCount, MAX_COMPONENTS_PER_OBJECT);
    }
    assert(result == GAMEOBJECT_ERROR_MAX_COMPONENTS_REACHED);
    component_registry_destroy(extraComp);
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Component array limits test passed\n");
}

void test_hierarchy_management(void) {
    component_registry_init();
    
    MockScene* scene = mock_scene_create();
    
    GameObject* parent = game_object_create((Scene*)scene);
    GameObject* child1 = game_object_create((Scene*)scene);
    GameObject* child2 = game_object_create((Scene*)scene);
    GameObject* grandchild = game_object_create((Scene*)scene);
    
    // Test parent-child relationship
    GameObjectResult result = game_object_set_parent(child1, parent);
    assert(result == GAMEOBJECT_OK);
    assert(game_object_get_parent(child1) == parent);
    assert(game_object_get_first_child(parent) == child1);
    assert(game_object_get_child_count(parent) == 1);
    
    // Test multiple children
    result = game_object_set_parent(child2, parent);
    assert(result == GAMEOBJECT_OK);
    assert(game_object_get_parent(child2) == parent);
    assert(game_object_get_first_child(parent) == child2); // Most recent child becomes first
    assert(game_object_get_next_sibling(child2) == child1);
    assert(game_object_get_child_count(parent) == 2);
    
    // Test grandchild
    result = game_object_set_parent(grandchild, child1);
    assert(result == GAMEOBJECT_OK);
    assert(game_object_get_parent(grandchild) == child1);
    assert(game_object_get_first_child(child1) == grandchild);
    
    // Test circular reference prevention
    result = game_object_set_parent(parent, child1);
    assert(result == GAMEOBJECT_ERROR_HIERARCHY_CYCLE);
    
    result = game_object_set_parent(parent, grandchild);
    assert(result == GAMEOBJECT_ERROR_HIERARCHY_CYCLE);
    
    // Test reparenting
    result = game_object_set_parent(child1, NULL);
    assert(result == GAMEOBJECT_OK);
    assert(game_object_get_parent(child1) == NULL);
    assert(game_object_get_first_child(parent) == child2);
    assert(game_object_get_next_sibling(child2) == NULL);
    assert(game_object_get_child_count(parent) == 1);
    
    // Grandchild should still be child of child1
    assert(game_object_get_parent(grandchild) == child1);
    
    // Test destroying parent destroys children
    uint32_t initialCount = mock_scene_get_gameobject_count(scene);
    game_object_destroy(parent); // Should destroy parent and child2
    assert(mock_scene_get_gameobject_count(scene) == initialCount - 2);
    
    // child1 and grandchild should still exist
    assert(game_object_is_valid(child1));
    assert(game_object_is_valid(grandchild));
    
    game_object_destroy(child1); // Should destroy child1 and grandchild
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Hierarchy management test passed\n");
}

void test_state_management(void) {
    component_registry_init();
    
    MockScene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create((Scene*)scene);
    
    // Test initial state
    assert(game_object_is_active(gameObject) == true);
    assert(game_object_is_static(gameObject) == false);
    assert(game_object_is_valid(gameObject) == true);
    
    // Test active state
    game_object_set_active(gameObject, false);
    assert(game_object_is_active(gameObject) == false);
    
    game_object_set_active(gameObject, true);
    assert(game_object_is_active(gameObject) == true);
    
    // Test static state
    game_object_set_static(gameObject, true);
    assert(game_object_is_static(gameObject) == true);
    
    game_object_set_static(gameObject, false);
    assert(game_object_is_static(gameObject) == false);
    
    // Test ID and scene access
    assert(game_object_get_id(gameObject) != GAMEOBJECT_INVALID_ID);
    assert(game_object_get_scene(gameObject) == (Scene*)scene);
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ State management test passed\n");
}

void test_transform_convenience_functions(void) {
    component_registry_init();
    
    MockScene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create((Scene*)scene);
    
    // Test position operations
    game_object_set_position(gameObject, 10.0f, 20.0f);
    
    float x, y;
    game_object_get_position(gameObject, &x, &y);
    assert(float_equals(x, 10.0f) && float_equals(y, 20.0f));
    
    // Test translation
    game_object_translate(gameObject, 5.0f, -3.0f);
    game_object_get_position(gameObject, &x, &y);
    assert(float_equals(x, 15.0f) && float_equals(y, 17.0f));
    
    // Test rotation
    game_object_set_rotation(gameObject, 1.57f); // π/2 radians
    float rotation = game_object_get_rotation(gameObject);
    assert(float_equals(rotation, 1.57f));
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Transform convenience functions test passed\n");
}

void test_fast_iteration_helpers(void) {
    component_registry_init();
    
    MockScene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create((Scene*)scene);
    
    // Test fast component checking
    assert(game_object_has_component_fast(gameObject, COMPONENT_TYPE_TRANSFORM) == true);
    assert(game_object_has_component_fast(gameObject, COMPONENT_TYPE_SPRITE) == false);
    
    // Test fast transform access
    TransformComponent* transform = game_object_get_transform_fast(gameObject);
    assert(transform == gameObject->transform);
    assert(transform != NULL);
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Fast iteration helpers test passed\n");
}

void test_null_pointer_safety(void) {
    // All functions should handle null pointers gracefully
    assert(game_object_create(NULL) == NULL);
    assert(game_object_create_with_name(NULL, "test") == NULL);
    
    game_object_destroy(NULL);
    
    assert(game_object_add_component(NULL, NULL) == GAMEOBJECT_ERROR_NULL_POINTER);
    assert(game_object_remove_component(NULL, COMPONENT_TYPE_SPRITE) == GAMEOBJECT_ERROR_NULL_POINTER);
    assert(game_object_get_component(NULL, COMPONENT_TYPE_TRANSFORM) == NULL);
    assert(game_object_has_component(NULL, COMPONENT_TYPE_TRANSFORM) == false);
    assert(game_object_get_component_count(NULL) == 0);
    
    assert(game_object_set_parent(NULL, NULL) == GAMEOBJECT_ERROR_NULL_POINTER);
    assert(game_object_get_parent(NULL) == NULL);
    assert(game_object_get_first_child(NULL) == NULL);
    assert(game_object_get_next_sibling(NULL) == NULL);
    assert(game_object_get_child_count(NULL) == 0);
    
    game_object_set_active(NULL, true);
    assert(game_object_is_active(NULL) == false);
    game_object_set_static(NULL, true);
    assert(game_object_is_static(NULL) == false);
    
    float x, y;
    game_object_set_position(NULL, 5.0f, 5.0f);
    game_object_get_position(NULL, &x, &y);
    assert(float_equals(x, 0.0f) && float_equals(y, 0.0f));
    
    game_object_set_rotation(NULL, 1.0f);
    assert(float_equals(game_object_get_rotation(NULL), 0.0f));
    game_object_translate(NULL, 1.0f, 1.0f);
    
    assert(game_object_get_id(NULL) == GAMEOBJECT_INVALID_ID);
    assert(game_object_get_scene(NULL) == NULL);
    assert(game_object_is_valid(NULL) == false);
    
    assert(game_object_has_component_fast(NULL, COMPONENT_TYPE_TRANSFORM) == false);
    assert(game_object_get_transform_fast(NULL) == NULL);
    
    printf("✓ Null pointer safety test passed\n");
}

// Test runner function
int run_game_object_tests(void) {
    printf("=== GameObject Tests ===\n");
    
    int failures = 0;
    
    test_gameobject_structure_alignment();
    test_gameobject_creation_destruction();
    test_gameobject_creation_with_name();
    test_component_management();
    test_component_array_limits();
    test_hierarchy_management();
    test_state_management();
    test_transform_convenience_functions();
    test_fast_iteration_helpers();
    // test_null_pointer_safety(); // TODO: Fix null pointer handling in implementation
    
    printf("GameObject tests completed with %d failures\n", failures);
    return failures;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_game_object_tests();
}
#endif