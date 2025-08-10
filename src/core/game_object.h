#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "component.h"
#include "memory_pool.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct Scene Scene;
typedef struct TransformComponent TransformComponent;

#define MAX_COMPONENTS_PER_OBJECT 4
#define GAMEOBJECT_INVALID_ID 0

// GameObject structure (96 bytes, cache-aligned) 
struct GameObject {
    uint32_t id;                           // 4 bytes - unique identifier
    uint32_t componentMask;                // 4 bytes - bitmask of attached components
    Component* components[MAX_COMPONENTS_PER_OBJECT]; // 32 bytes - component array
    TransformComponent* transform;         // 8 bytes - cached transform pointer
    Scene* scene;                         // 8 bytes - parent scene
    GameObject* parent;                   // 8 bytes - parent GameObject
    GameObject* firstChild;               // 8 bytes - first child in hierarchy
    GameObject* nextSibling;              // 8 bytes - next sibling GameObject
    uint8_t active;                       // 1 byte - active state
    uint8_t staticObject;                 // 1 byte - optimization hint
    uint8_t componentCount;               // 1 byte - number of attached components
    uint8_t padding[13];                  // 13 bytes - explicit padding for 96-byte alignment
};

// GameObject creation and destruction
typedef enum {
    GAMEOBJECT_OK = 0,
    GAMEOBJECT_ERROR_NULL_POINTER,
    GAMEOBJECT_ERROR_OUT_OF_MEMORY,
    GAMEOBJECT_ERROR_COMPONENT_NOT_FOUND,
    GAMEOBJECT_ERROR_COMPONENT_ALREADY_EXISTS,
    GAMEOBJECT_ERROR_MAX_COMPONENTS_REACHED,
    GAMEOBJECT_ERROR_INVALID_COMPONENT_TYPE,
    GAMEOBJECT_ERROR_HIERARCHY_CYCLE
} GameObjectResult;

// GameObject lifecycle
GameObject* game_object_create(Scene* scene);
GameObject* game_object_create_with_name(Scene* scene, const char* debugName);
void game_object_destroy(GameObject* gameObject);

// Component management
GameObjectResult game_object_add_component(GameObject* gameObject, Component* component);
GameObjectResult game_object_remove_component(GameObject* gameObject, ComponentType type);
Component* game_object_get_component(GameObject* gameObject, ComponentType type);
bool game_object_has_component(GameObject* gameObject, ComponentType type);
uint32_t game_object_get_component_count(GameObject* gameObject);

// Hierarchy management
GameObjectResult game_object_set_parent(GameObject* child, GameObject* parent);
GameObject* game_object_get_parent(GameObject* gameObject);
GameObject* game_object_get_first_child(GameObject* gameObject);
GameObject* game_object_get_next_sibling(GameObject* gameObject);
uint32_t game_object_get_child_count(GameObject* gameObject);

// State management
void game_object_set_active(GameObject* gameObject, bool active);
bool game_object_is_active(GameObject* gameObject);
void game_object_set_static(GameObject* gameObject, bool staticObject);
bool game_object_is_static(GameObject* gameObject);

// Transform convenience functions (direct access to transform component)
void game_object_set_position(GameObject* gameObject, float x, float y);
void game_object_get_position(GameObject* gameObject, float* x, float* y);
void game_object_set_rotation(GameObject* gameObject, float rotation);
float game_object_get_rotation(GameObject* gameObject);
void game_object_translate(GameObject* gameObject, float dx, float dy);

// Queries and utilities
uint32_t game_object_get_id(GameObject* gameObject);
Scene* game_object_get_scene(GameObject* gameObject);
bool game_object_is_valid(GameObject* gameObject);

// Fast iteration helpers (for batch operations)
static inline bool game_object_has_component_fast(GameObject* gameObject, ComponentType type) {
    return (gameObject->componentMask & type) != 0;
}

static inline TransformComponent* game_object_get_transform_fast(GameObject* gameObject) {
    return gameObject->transform;
}

#endif // GAME_OBJECT_H