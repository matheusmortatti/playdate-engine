#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "../core/component.h"

// Transform component structure (64 bytes)
typedef struct TransformComponent {
    Component base;                // 48 bytes - base component
    float x, y;                   // 8 bytes - position
    float rotation;               // 4 bytes - rotation in radians
    float scaleX, scaleY;         // 8 bytes - scale factors
    float matrix[6];              // 24 bytes - cached 2D transformation matrix [a,b,c,d,tx,ty]
    bool matrixDirty;            // 1 byte - needs matrix recalculation
    uint8_t padding[7];          // 7 bytes - alignment padding to reach 64 bytes
} TransformComponent;

// Transform component interface
TransformComponent* transform_component_create(GameObject* gameObject);
void transform_component_destroy(TransformComponent* transform);

// Position operations
void transform_component_set_position(TransformComponent* transform, float x, float y);
void transform_component_get_position(const TransformComponent* transform, float* x, float* y);
void transform_component_translate(TransformComponent* transform, float dx, float dy);

// Rotation operations
void transform_component_set_rotation(TransformComponent* transform, float rotation);
float transform_component_get_rotation(const TransformComponent* transform);
void transform_component_rotate(TransformComponent* transform, float deltaRotation);

// Scale operations
void transform_component_set_scale(TransformComponent* transform, float scaleX, float scaleY);
void transform_component_get_scale(const TransformComponent* transform, float* scaleX, float* scaleY);

// Matrix operations
const float* transform_component_get_matrix(TransformComponent* transform);
void transform_component_calculate_matrix(TransformComponent* transform);
void transform_component_mark_dirty(TransformComponent* transform);

// Utility functions
void transform_component_look_at(TransformComponent* transform, float targetX, float targetY);
void transform_component_transform_point(const TransformComponent* transform, 
                                       float localX, float localY, 
                                       float* worldX, float* worldY);

#endif // TRANSFORM_COMPONENT_H