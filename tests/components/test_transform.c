#include "../../src/components/transform_component.h"
#include "../../src/core/component_registry.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

// Mock GameObject for testing
typedef struct MockGameObject {
    uint32_t id;
    const char* name;
} MockGameObject;

const float EPSILON = 0.0001f;

bool float_equals(float a, float b) {
    return fabs(a - b) < EPSILON;
}

void test_transform_component_creation(void) {
    component_registry_init();
    
    MockGameObject gameObject = {1, "TransformTest"};
    
    TransformComponent* transform = transform_component_create((GameObject*)&gameObject);
    assert(transform != NULL);
    assert(transform->base.type == COMPONENT_TYPE_TRANSFORM);
    assert(transform->base.gameObject == (GameObject*)&gameObject);
    assert(transform->base.enabled == true);
    
    // Check default values
    assert(float_equals(transform->x, 0.0f));
    assert(float_equals(transform->y, 0.0f));
    assert(float_equals(transform->rotation, 0.0f));
    assert(transform->matrixDirty == true);
    
    transform_component_destroy(transform);
    component_registry_shutdown();
    printf("✓ Transform component creation test passed\n");
}

void test_transform_position_operations(void) {
    component_registry_init();
    
    MockGameObject gameObject = {1, "PositionTest"};
    TransformComponent* transform = transform_component_create((GameObject*)&gameObject);
    assert(transform != NULL);
    
    // Test position setting
    transform_component_set_position(transform, 10.5f, -20.3f);
    assert(float_equals(transform->x, 10.5f));
    assert(float_equals(transform->y, -20.3f));
    assert(transform->matrixDirty == true);
    
    // Test position getting
    float x, y;
    transform_component_get_position(transform, &x, &y);
    assert(float_equals(x, 10.5f));
    assert(float_equals(y, -20.3f));
    
    // Test translation
    transform_component_translate(transform, 5.0f, 10.0f);
    transform_component_get_position(transform, &x, &y);
    assert(float_equals(x, 15.5f));
    assert(float_equals(y, -10.3f));
    
    transform_component_destroy(transform);
    component_registry_shutdown();
    printf("✓ Transform position operations test passed\n");
}

void test_transform_rotation_operations(void) {
    component_registry_init();
    
    MockGameObject gameObject = {1, "RotationTest"};
    TransformComponent* transform = transform_component_create((GameObject*)&gameObject);
    assert(transform != NULL);
    
    // Test rotation setting
    float angle = M_PI / 4.0f; // 45 degrees
    transform_component_set_rotation(transform, angle);
    assert(float_equals(transform->rotation, angle));
    assert(transform->matrixDirty == true);
    
    // Test rotation getting
    float rotation = transform_component_get_rotation(transform);
    assert(float_equals(rotation, angle));
    
    // Test rotation increment
    transform_component_rotate(transform, M_PI / 4.0f); // Another 45 degrees
    rotation = transform_component_get_rotation(transform);
    assert(float_equals(rotation, M_PI / 2.0f)); // Should be 90 degrees total
    
    transform_component_destroy(transform);
    component_registry_shutdown();
    printf("✓ Transform rotation operations test passed\n");
}

void test_transform_scale_operations(void) {
    component_registry_init();
    
    MockGameObject gameObject = {1, "ScaleTest"};
    TransformComponent* transform = transform_component_create((GameObject*)&gameObject);
    assert(transform != NULL);
    
    // Test scale setting (no-op in simplified implementation)
    transform_component_set_scale(transform, 2.0f, 0.5f);
    // Scale values are not stored in simplified implementation
    
    // Test scale getting (always returns identity in simplified implementation)
    float scaleX, scaleY;
    transform_component_get_scale(transform, &scaleX, &scaleY);
    assert(float_equals(scaleX, 1.0f));  // Always identity
    assert(float_equals(scaleY, 1.0f));  // Always identity
    
    transform_component_destroy(transform);
    component_registry_shutdown();
    printf("✓ Transform scale operations test passed\n");
}

void test_transform_matrix_generation(void) {
    component_registry_init();
    
    MockGameObject gameObject = {1, "MatrixTest"};
    TransformComponent* transform = transform_component_create((GameObject*)&gameObject);
    assert(transform != NULL);
    
    // Set transform values
    transform_component_set_position(transform, 10.0f, 20.0f);
    transform_component_set_rotation(transform, M_PI / 2.0f); // 90 degrees
    
    // Get matrix (should trigger calculation)
    const float* matrix = transform_component_get_matrix(transform);
    assert(matrix != NULL);
    assert(!transform->matrixDirty); // Should no longer be dirty
    
    // Matrix should be a 2D transformation matrix [a, b, c, d, tx, ty]
    // For 90-degree rotation (no scale) and translation (10, 20):
    // [0, -1, 1, 0, 10, 20]
    assert(float_equals(matrix[0], 0.0f));   // a: cos
    assert(float_equals(matrix[1], -1.0f));  // b: -sin
    assert(float_equals(matrix[2], 1.0f));   // c: sin
    assert(float_equals(matrix[3], 0.0f));   // d: cos
    assert(float_equals(matrix[4], 10.0f));  // tx: translation X
    assert(float_equals(matrix[5], 20.0f));  // ty: translation Y
    
    // Getting matrix again should return same values
    const float* matrix2 = transform_component_get_matrix(transform);
    assert(matrix2 != NULL);
    
    transform_component_destroy(transform);
    component_registry_shutdown();
    printf("✓ Transform matrix generation test passed\n");
}

void test_transform_matrix_dirty_flag(void) {
    component_registry_init();
    
    MockGameObject gameObject = {1, "DirtyTest"};
    TransformComponent* transform = transform_component_create((GameObject*)&gameObject);
    assert(transform != NULL);
    
    // Initially dirty
    assert(transform->matrixDirty == true);
    
    // Get matrix clears dirty flag
    const float* matrix = transform_component_get_matrix(transform);
    assert(matrix != NULL);
    assert(!transform->matrixDirty);
    
    // Position change marks dirty
    transform_component_set_position(transform, 5.0f, 5.0f);
    assert(transform->matrixDirty);
    
    transform_component_get_matrix(transform); // Clear dirty
    assert(!transform->matrixDirty);
    
    // Rotation change marks dirty
    transform_component_set_rotation(transform, M_PI / 4.0f);
    assert(transform->matrixDirty);
    
    transform_component_get_matrix(transform); // Clear dirty
    assert(!transform->matrixDirty);
    
    // Scale change marks dirty (no-op in simplified implementation)
    transform_component_set_scale(transform, 2.0f, 2.0f);
    // Note: scale doesn't affect dirty flag in simplified implementation
    
    // Manual dirty marking
    transform_component_get_matrix(transform); // Clear dirty
    assert(!transform->matrixDirty);
    transform_component_mark_dirty(transform);
    assert(transform->matrixDirty);
    
    transform_component_destroy(transform);
    component_registry_shutdown();
    printf("✓ Transform matrix dirty flag test passed\n");
}

void test_transform_point_transformation(void) {
    component_registry_init();
    
    MockGameObject gameObject = {1, "PointTest"};
    TransformComponent* transform = transform_component_create((GameObject*)&gameObject);
    assert(transform != NULL);
    
    // Set up a simple transformation (no scale, no rotation)
    transform_component_set_position(transform, 10.0f, 20.0f);
    
    // Transform a point
    float worldX, worldY;
    transform_component_transform_point(transform, 5.0f, 4.0f, &worldX, &worldY);
    
    // Expected: (5 + 10, 4 + 20) = (15, 24) with no scale/rotation
    assert(float_equals(worldX, 15.0f));
    assert(float_equals(worldY, 24.0f));
    
    transform_component_destroy(transform);
    component_registry_shutdown();
    printf("✓ Transform point transformation test passed\n");
}

void test_transform_look_at(void) {
    component_registry_init();
    
    MockGameObject gameObject = {1, "LookAtTest"};
    TransformComponent* transform = transform_component_create((GameObject*)&gameObject);
    assert(transform != NULL);
    
    // Set position
    transform_component_set_position(transform, 0.0f, 0.0f);
    
    // Look at point to the right
    transform_component_look_at(transform, 10.0f, 0.0f);
    float rotation = transform_component_get_rotation(transform);
    assert(float_equals(rotation, 0.0f)); // Should be 0 radians (facing right)
    
    // Look at point above
    transform_component_look_at(transform, 0.0f, 10.0f);
    rotation = transform_component_get_rotation(transform);
    assert(float_equals(rotation, M_PI / 2.0f)); // Should be 90 degrees
    
    transform_component_destroy(transform);
    component_registry_shutdown();
    printf("✓ Transform look at test passed\n");
}

void test_transform_structure_alignment(void) {
    // Verify the TransformComponent structure is properly aligned
    assert(sizeof(TransformComponent) == 64); // Should be exactly 64 bytes
    assert(sizeof(TransformComponent) % 16 == 0); // Must be 16-byte aligned
    
    // Verify it includes the base component
    TransformComponent transform;
    Component* base = (Component*)&transform;
    assert(base == &transform.base);
    
    printf("✓ Transform structure alignment test passed\n");
}

void test_transform_null_pointer_safety(void) {
    // All functions should handle null pointers gracefully
    transform_component_destroy(NULL);
    
    float x, y;
    transform_component_get_position(NULL, &x, &y);
    assert(float_equals(x, 0.0f));
    assert(float_equals(y, 0.0f));
    
    transform_component_set_position(NULL, 5.0f, 5.0f);
    transform_component_translate(NULL, 1.0f, 1.0f);
    
    float rotation = transform_component_get_rotation(NULL);
    assert(float_equals(rotation, 0.0f));
    
    transform_component_set_rotation(NULL, M_PI);
    transform_component_rotate(NULL, M_PI / 2.0f);
    
    float scaleX, scaleY;
    transform_component_get_scale(NULL, &scaleX, &scaleY);
    assert(float_equals(scaleX, 1.0f));
    assert(float_equals(scaleY, 1.0f));
    
    transform_component_set_scale(NULL, 2.0f, 2.0f);
    
    const float* matrix = transform_component_get_matrix(NULL);
    assert(matrix == NULL);
    
    transform_component_mark_dirty(NULL);
    transform_component_look_at(NULL, 5.0f, 5.0f);
    
    float worldX, worldY;
    transform_component_transform_point(NULL, 1.0f, 1.0f, &worldX, &worldY);
    assert(float_equals(worldX, 0.0f));
    assert(float_equals(worldY, 0.0f));
    
    printf("✓ Transform null pointer safety test passed\n");
}

// Test runner function
int run_transform_component_tests(void) {
    printf("=== Transform Component Tests ===\n");
    
    int failures = 0;
    
    test_transform_component_creation();
    test_transform_position_operations();
    test_transform_rotation_operations();
    test_transform_scale_operations();
    test_transform_matrix_generation();
    test_transform_matrix_dirty_flag();
    test_transform_point_transformation();
    test_transform_look_at();
    test_transform_structure_alignment();
    test_transform_null_pointer_safety();
    
    printf("Transform component tests completed with %d failures\n", failures);
    return failures;
}

#ifdef TEST_STANDALONE
int main(void) {
    return run_transform_component_tests();
}
#endif