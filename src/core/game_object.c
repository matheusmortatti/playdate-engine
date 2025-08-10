#include "game_object.h"
#include "../components/transform_component.h"
#include "component_registry.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static uint32_t g_nextGameObjectId = 1;

// Forward declarations for mock scene integration
ObjectPool* scene_get_gameobject_pool(Scene* scene);
void scene_add_game_object(Scene* scene, GameObject* gameObject);
void scene_remove_game_object(Scene* scene, GameObject* gameObject);

GameObject* game_object_create(Scene* scene) {
    return game_object_create_with_name(scene, NULL);
}

GameObject* game_object_create_with_name(Scene* scene, const char* debugName) {
    (void)debugName; // Unused in this implementation
    
    if (!scene) {
        return NULL;
    }
    
    // Allocate GameObject from scene's object pool
    GameObject* gameObject = (GameObject*)object_pool_alloc(scene_get_gameobject_pool(scene));
    if (!gameObject) {
        return NULL;
    }
    
    // Initialize GameObject
    memset(gameObject, 0, sizeof(GameObject));
    gameObject->id = g_nextGameObjectId++;
    gameObject->scene = scene;
    gameObject->active = true;
    gameObject->staticObject = false;
    gameObject->componentCount = 0;
    gameObject->componentMask = 0;
    
    // Every GameObject must have a TransformComponent
    TransformComponent* transform = transform_component_create(gameObject);
    if (!transform) {
        object_pool_free(scene_get_gameobject_pool(scene), gameObject);
        return NULL;
    }
    
    gameObject->transform = transform;
    gameObject->components[0] = (Component*)transform;
    gameObject->componentMask |= COMPONENT_TYPE_TRANSFORM;
    gameObject->componentCount = 1;
    
    // Register with scene
    scene_add_game_object(scene, gameObject);
    
    return gameObject;
}

void game_object_destroy(GameObject* gameObject) {
    if (!gameObject) return;
    
    // Destroy all children first
    GameObject* child = gameObject->firstChild;
    while (child) {
        GameObject* nextChild = child->nextSibling;
        game_object_destroy(child);
        child = nextChild;
    }
    
    // Remove from parent's child list
    if (gameObject->parent) {
        game_object_set_parent(gameObject, NULL);
    }
    
    // Destroy all components
    for (uint32_t i = 0; i < gameObject->componentCount; i++) {
        Component* component = gameObject->components[i];
        if (component) {
            component_registry_destroy(component);
        }
    }
    
    // Remove from scene
    scene_remove_game_object(gameObject->scene, gameObject);
    
    // Return to pool
    object_pool_free(scene_get_gameobject_pool(gameObject->scene), gameObject);
}

GameObjectResult game_object_add_component(GameObject* gameObject, Component* component) {
    if (!gameObject || !component) {
        return GAMEOBJECT_ERROR_NULL_POINTER;
    }
    
    ComponentType type = component->type;
    
    // Check if component type already exists
    if (game_object_has_component(gameObject, type)) {
        return GAMEOBJECT_ERROR_COMPONENT_ALREADY_EXISTS;
    }
    
    // Check if we have space for more components
    if (gameObject->componentCount >= MAX_COMPONENTS_PER_OBJECT) {
        return GAMEOBJECT_ERROR_MAX_COMPONENTS_REACHED;
    }
    
    // Add component to array
    gameObject->components[gameObject->componentCount] = component;
    gameObject->componentCount++;
    
    // Update component mask
    gameObject->componentMask |= type;
    
    // Cache commonly used components
    if (type == COMPONENT_TYPE_TRANSFORM) {
        gameObject->transform = (TransformComponent*)component;
    }
    
    return GAMEOBJECT_OK;
}

GameObjectResult game_object_remove_component(GameObject* gameObject, ComponentType type) {
    if (!gameObject) {
        return GAMEOBJECT_ERROR_NULL_POINTER;
    }
    
    // Cannot remove transform component
    if (type == COMPONENT_TYPE_TRANSFORM) {
        return GAMEOBJECT_ERROR_INVALID_COMPONENT_TYPE;
    }
    
    // Find component in array
    for (uint32_t i = 0; i < gameObject->componentCount; i++) {
        Component* component = gameObject->components[i];
        if (component && (component->type & type)) {
            // Destroy component
            component_registry_destroy(component);
            
            // Remove from array (shift remaining components)
            for (uint32_t j = i; j < gameObject->componentCount - 1; j++) {
                gameObject->components[j] = gameObject->components[j + 1];
            }
            gameObject->components[gameObject->componentCount - 1] = NULL;
            gameObject->componentCount--;
            
            // Update component mask
            gameObject->componentMask &= ~type;
            
            return GAMEOBJECT_OK;
        }
    }
    
    return GAMEOBJECT_ERROR_COMPONENT_NOT_FOUND;
}

Component* game_object_get_component(GameObject* gameObject, ComponentType type) {
    if (!gameObject) return NULL;
    
    // Fast path for transform component
    if (type == COMPONENT_TYPE_TRANSFORM) {
        return (Component*)gameObject->transform;
    }
    
    // Search component array
    for (uint32_t i = 0; i < gameObject->componentCount; i++) {
        Component* component = gameObject->components[i];
        if (component && (component->type & type)) {
            return component;
        }
    }
    
    return NULL;
}

bool game_object_has_component(GameObject* gameObject, ComponentType type) {
    return gameObject ? (gameObject->componentMask & type) != 0 : false;
}

uint32_t game_object_get_component_count(GameObject* gameObject) {
    return gameObject ? gameObject->componentCount : 0;
}

GameObjectResult game_object_set_parent(GameObject* child, GameObject* parent) {
    if (!child) {
        return GAMEOBJECT_ERROR_NULL_POINTER;
    }
    
    // Check for circular reference
    GameObject* current = parent;
    while (current) {
        if (current == child) {
            return GAMEOBJECT_ERROR_HIERARCHY_CYCLE;
        }
        current = current->parent;
    }
    
    // Remove from current parent
    if (child->parent) {
        GameObject* oldParent = child->parent;
        
        if (oldParent->firstChild == child) {
            oldParent->firstChild = child->nextSibling;
        } else {
            GameObject* sibling = oldParent->firstChild;
            while (sibling && sibling->nextSibling != child) {
                sibling = sibling->nextSibling;
            }
            if (sibling) {
                sibling->nextSibling = child->nextSibling;
            }
        }
    }
    
    // Set new parent
    child->parent = parent;
    child->nextSibling = NULL;
    
    if (parent) {
        // Add to parent's child list
        child->nextSibling = parent->firstChild;
        parent->firstChild = child;
    }
    
    return GAMEOBJECT_OK;
}

GameObject* game_object_get_parent(GameObject* gameObject) {
    return gameObject ? gameObject->parent : NULL;
}

GameObject* game_object_get_first_child(GameObject* gameObject) {
    return gameObject ? gameObject->firstChild : NULL;
}

GameObject* game_object_get_next_sibling(GameObject* gameObject) {
    return gameObject ? gameObject->nextSibling : NULL;
}

uint32_t game_object_get_child_count(GameObject* gameObject) {
    if (!gameObject) return 0;
    
    uint32_t count = 0;
    GameObject* child = gameObject->firstChild;
    while (child) {
        count++;
        child = child->nextSibling;
    }
    return count;
}

void game_object_set_active(GameObject* gameObject, bool active) {
    if (gameObject) {
        gameObject->active = active ? 1 : 0;
    }
}

bool game_object_is_active(GameObject* gameObject) {
    return gameObject ? gameObject->active != 0 : false;
}

void game_object_set_static(GameObject* gameObject, bool staticObject) {
    if (gameObject) {
        gameObject->staticObject = staticObject ? 1 : 0;
    }
}

bool game_object_is_static(GameObject* gameObject) {
    return gameObject ? gameObject->staticObject != 0 : false;
}

void game_object_set_position(GameObject* gameObject, float x, float y) {
    if (gameObject && gameObject->transform) {
        transform_component_set_position(gameObject->transform, x, y);
    }
}

void game_object_get_position(GameObject* gameObject, float* x, float* y) {
    if (gameObject && gameObject->transform) {
        transform_component_get_position(gameObject->transform, x, y);
    } else {
        if (x) *x = 0.0f;
        if (y) *y = 0.0f;
    }
}

void game_object_set_rotation(GameObject* gameObject, float rotation) {
    if (gameObject && gameObject->transform) {
        transform_component_set_rotation(gameObject->transform, rotation);
    }
}

float game_object_get_rotation(GameObject* gameObject) {
    if (gameObject && gameObject->transform) {
        return transform_component_get_rotation(gameObject->transform);
    }
    return 0.0f;
}

void game_object_translate(GameObject* gameObject, float dx, float dy) {
    if (gameObject && gameObject->transform) {
        transform_component_translate(gameObject->transform, dx, dy);
    }
}

uint32_t game_object_get_id(GameObject* gameObject) {
    return gameObject ? gameObject->id : GAMEOBJECT_INVALID_ID;
}

Scene* game_object_get_scene(GameObject* gameObject) {
    return gameObject ? gameObject->scene : NULL;
}

bool game_object_is_valid(GameObject* gameObject) {
    return gameObject != NULL && gameObject->id != GAMEOBJECT_INVALID_ID;
}