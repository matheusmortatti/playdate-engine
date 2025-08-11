#ifndef UPDATE_SYSTEMS_H
#define UPDATE_SYSTEMS_H

#include "component.h"
#include "scene.h"

// Transform system
void transform_system_update_batch(Component** components, uint32_t count, float deltaTime);

// Sprite system  
void sprite_system_update_batch(Component** components, uint32_t count, float deltaTime);
void sprite_system_render_batch(Component** components, uint32_t count);

// Collision system
void collision_system_update_batch(Component** components, uint32_t count, float deltaTime);

// System registration helper
void register_default_systems(Scene* scene);

#endif // UPDATE_SYSTEMS_H