#include "transform_component.h"
#include "../core/component_registry.h"
#include <math.h>
#include <string.h>

// Forward declarations for vtable functions  
static void transform_init(Component* component, GameObject* gameObject);
static void transform_destroy(Component* component);
static void transform_update(Component* component, float deltaTime);

// Transform component vtable
static const ComponentVTable transformVTable = {
    .init = transform_init,
    .destroy = transform_destroy,
    .clone = NULL,
    .update = transform_update,
    .fixedUpdate = NULL,
    .render = NULL,
    .onEnabled = NULL,
    .onDisabled = NULL,
    .onGameObjectDestroyed = NULL,
    .getSerializedSize = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// 2D transformation matrix calculation with scale support
static void calculate_matrix(TransformComponent* transform, float* matrix) {  
    if (!transform || !matrix) return;
    
    float cos_r = cosf(transform->rotation);
    float sin_r = sinf(transform->rotation);
    
    // 2D transformation matrix: [a, b, c, d, tx, ty]
    // | a  c  tx |   | scaleX*cos  -scaleY*sin  x |
    // | b  d  ty | = | scaleX*sin   scaleY*cos  y |
    // | 0  0  1  |   |     0            0       1 |
    
    matrix[0] = transform->scaleX * cos_r;  // a
    matrix[1] = transform->scaleX * sin_r;  // b
    matrix[2] = -transform->scaleY * sin_r; // c
    matrix[3] = transform->scaleY * cos_r;  // d
    matrix[4] = transform->x;               // tx
    matrix[5] = transform->y;               // ty
}

// VTable implementations
static void transform_init(Component* component, GameObject* gameObject) {
    (void)gameObject; // Unused in this implementation
    
    if (!component) return;
    
    TransformComponent* transform = (TransformComponent*)component;
    
    // Initialize transform values to identity
    transform->x = 0.0f;
    transform->y = 0.0f;
    transform->rotation = 0.0f;
    transform->scaleX = 1.0f;
    transform->scaleY = 1.0f;
    transform->matrixDirty = true;
    
    // Initialize matrix to identity
    memset(transform->matrix, 0, sizeof(transform->matrix));
    transform->matrix[0] = 1.0f; // a
    transform->matrix[3] = 1.0f; // d
}

static void transform_destroy(Component* component) {
    if (!component) return;
    
    // Component-specific cleanup only - do NOT zero the base Component structure
    // as the registry needs the type field to return it to the correct pool
    TransformComponent* transform = (TransformComponent*)component;
    
    // Zero only the transform-specific fields, not the base component
    transform->x = 0.0f;
    transform->y = 0.0f;
    transform->rotation = 0.0f;
    transform->scaleX = 1.0f;
    transform->scaleY = 1.0f;
    transform->matrixDirty = false;
    memset(transform->matrix, 0, sizeof(transform->matrix));
    
    // Base component cleanup is handled by component_registry_destroy()
}

static void transform_update(Component* component, float deltaTime) {
    (void)component;   // No per-frame update logic needed
    (void)deltaTime;   // Transform is updated on-demand
}

// Public API implementations
TransformComponent* transform_component_create(GameObject* gameObject) {
    if (!gameObject) return NULL;
    
    // Ensure transform component type is registered
    if (!component_registry_is_type_registered(COMPONENT_TYPE_TRANSFORM)) {
        // Register the transform component type with default settings
        ComponentResult result = component_registry_register_type(
            COMPONENT_TYPE_TRANSFORM,
            sizeof(TransformComponent),
            DEFAULT_COMPONENT_POOL_SIZE,
            &transformVTable,
            "Transform"
        );
        
        if (result != COMPONENT_OK) {
            return NULL;
        }
    }
    
    Component* component = component_registry_create(COMPONENT_TYPE_TRANSFORM, gameObject);
    return (TransformComponent*)component;
}

void transform_component_destroy(TransformComponent* transform) {
    if (!transform) return;
    
    component_registry_destroy((Component*)transform);
}

void transform_component_set_position(TransformComponent* transform, float x, float y) {
    if (!transform) return;
    
    transform->x = x;
    transform->y = y;
    transform->matrixDirty = true;
}

void transform_component_get_position(const TransformComponent* transform, float* x, float* y) {
    if (!transform) {
        if (x) *x = 0.0f;
        if (y) *y = 0.0f;
        return;
    }
    
    if (x) *x = transform->x;
    if (y) *y = transform->y;
}

void transform_component_translate(TransformComponent* transform, float dx, float dy) {
    if (!transform) return;
    
    transform->x += dx;
    transform->y += dy;
    transform->matrixDirty = true;
}

void transform_component_set_rotation(TransformComponent* transform, float rotation) {
    if (!transform) return;
    
    transform->rotation = rotation;
    transform->matrixDirty = true;
}

float transform_component_get_rotation(const TransformComponent* transform) {
    return transform ? transform->rotation : 0.0f;
}

void transform_component_rotate(TransformComponent* transform, float deltaRotation) {
    if (!transform) return;
    
    transform->rotation += deltaRotation;
    transform->matrixDirty = true;
}

void transform_component_set_scale(TransformComponent* transform, float scaleX, float scaleY) {
    if (!transform) return;
    
    transform->scaleX = scaleX;
    transform->scaleY = scaleY;
    transform->matrixDirty = true;
}

void transform_component_get_scale(const TransformComponent* transform, float* scaleX, float* scaleY) {
    if (!transform) {
        if (scaleX) *scaleX = 1.0f;
        if (scaleY) *scaleY = 1.0f;
        return;
    }
    
    if (scaleX) *scaleX = transform->scaleX;
    if (scaleY) *scaleY = transform->scaleY;
}

const float* transform_component_get_matrix(TransformComponent* transform) {
    if (!transform) return NULL;
    
    if (transform->matrixDirty) {
        transform_component_calculate_matrix(transform);
    }
    
    return transform->matrix;
}

void transform_component_calculate_matrix(TransformComponent* transform) {
    if (!transform) return;
    
    calculate_matrix(transform, transform->matrix);
    transform->matrixDirty = false;
}

void transform_component_mark_dirty(TransformComponent* transform) {
    if (!transform) return;
    
    transform->matrixDirty = true;
}

void transform_component_look_at(TransformComponent* transform, float targetX, float targetY) {
    if (!transform) return;
    
    float dx = targetX - transform->x;
    float dy = targetY - transform->y;
    
    transform->rotation = atan2f(dy, dx);
    transform->matrixDirty = true;
}

void transform_component_transform_point(const TransformComponent* transform, 
                                       float localX, float localY, 
                                       float* worldX, float* worldY) {
    if (!transform || !worldX || !worldY) return;
    
    // Ensure matrix is up to date
    if (transform->matrixDirty) {
        transform_component_calculate_matrix((TransformComponent*)transform);
    }
    
    const float* m = transform->matrix;
    
    *worldX = m[0] * localX + m[2] * localY + m[4];
    *worldY = m[1] * localX + m[3] * localY + m[5];
}