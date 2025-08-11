#include "update_systems.h"
#include "../components/transform_component.h"
#include <assert.h>

void transform_system_update_batch(Component** components, uint32_t count, float deltaTime) {
    if (!components || count == 0) return;
    
    // Batch update all transform components
    for (uint32_t i = 0; i < count; i++) {
        Component* component = components[i];
        if (component && component->enabled && component->type == COMPONENT_TYPE_TRANSFORM) {
            // Update transform component using virtual function call
            component_call_update(component, deltaTime);
            
            // For transforms, also update matrix if needed
            TransformComponent* transform = (TransformComponent*)component;
            if (transform->matrixDirty) {
                transform_component_calculate_matrix(transform);
            }
        }
    }
}

void sprite_system_update_batch(Component** components, uint32_t count, float deltaTime) {
    if (!components || count == 0) return;
    
    // Update sprite animations, visibility, etc.
    for (uint32_t i = 0; i < count; i++) {
        Component* component = components[i];
        if (component && component->enabled && component->type == COMPONENT_TYPE_SPRITE) {
            // Call component's update function if it exists
            component_call_update(component, deltaTime);
        }
    }
}

void sprite_system_render_batch(Component** components, uint32_t count) {
    if (!components || count == 0) return;
    
    // Batch render all visible sprites
    // Note: In a real implementation, this would be optimized with:
    // - Z-order sorting
    // - Frustum culling
    // - Texture atlas batching
    // - Draw call minimization
    
    for (uint32_t i = 0; i < count; i++) {
        Component* component = components[i];
        if (component && component->enabled && component->type == COMPONENT_TYPE_SPRITE) {
            // Get the transform for positioning
            GameObject* gameObject = component->gameObject;
            if (gameObject && gameObject->transform) {
                TransformComponent* transform = gameObject->transform;
                
                // Ensure transform matrix is up to date
                if (transform->matrixDirty) {
                    transform_component_calculate_matrix(transform);
                }
                
                // Call component's render function
                component_call_render(component);
            }
        }
    }
}

void collision_system_update_batch(Component** components, uint32_t count, float deltaTime) {
    if (!components || count == 0) return;
    
    // Update collision bounds, spatial partitioning, etc.
    // Phase 1: Update all collision component bounds
    for (uint32_t i = 0; i < count; i++) {
        Component* component = components[i];
        if (component && component->enabled && component->type == COMPONENT_TYPE_COLLISION) {
            // Update collision component
            component_call_update(component, deltaTime);
        }
    }
    
    // Phase 2: Broad-phase collision detection (will be implemented in Phase 5)
    // This would involve:
    // - Spatial partitioning updates
    // - AABB collision checks
    // - Narrow-phase collision detection
    
    // Phase 3: Collision resolution (will be implemented in Phase 5)
    // This would handle:
    // - Collision response
    // - Trigger events
    // - Physics integration
}

void register_default_systems(Scene* scene) {
    if (!scene) return;
    
    // Register transform system with highest priority (0)
    // Transforms need to be updated before other components that depend on position
    scene_register_component_system(scene, COMPONENT_TYPE_TRANSFORM,
                                   transform_system_update_batch, NULL, 0);
    
    // Register sprite system with medium priority (1)
    // Sprites depend on transform data for positioning
    scene_register_component_system(scene, COMPONENT_TYPE_SPRITE,
                                   sprite_system_update_batch, sprite_system_render_batch, 1);
    
    // Register collision system with lower priority (2)
    // Collision detection can happen after transforms are updated
    scene_register_component_system(scene, COMPONENT_TYPE_COLLISION,
                                   collision_system_update_batch, NULL, 2);
}