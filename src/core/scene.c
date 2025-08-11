#include "scene.h"
#include "component_registry.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

static uint32_t g_nextSceneId = 1;

// Scene lifecycle
Scene* scene_create(const char* name, uint32_t maxGameObjects) {
    if (maxGameObjects == 0) {
        return NULL;
    }
    
    Scene* scene = malloc(sizeof(Scene));
    if (!scene) {
        return NULL;
    }
    
    memset(scene, 0, sizeof(Scene));
    
    // Initialize basic properties
    scene->id = g_nextSceneId++;
    strncpy(scene->name, name ? name : "UnnamedScene", sizeof(scene->name) - 1);
    scene->name[sizeof(scene->name) - 1] = '\0'; // Ensure null termination
    scene->state = SCENE_STATE_INACTIVE;
    scene->timeScale = 1.0f;
    scene->gameObjectCapacity = maxGameObjects;
    
    // Allocate GameObject array
    scene->gameObjects = malloc(maxGameObjects * sizeof(GameObject*));
    if (!scene->gameObjects) {
        free(scene);
        return NULL;
    }
    memset(scene->gameObjects, 0, maxGameObjects * sizeof(GameObject*));
    
    // Initialize root objects array
    scene->rootObjectCapacity = maxGameObjects / 4; // Assume 25% are root objects
    if (scene->rootObjectCapacity < 10) scene->rootObjectCapacity = 10; // Minimum capacity
    scene->rootObjects = malloc(scene->rootObjectCapacity * sizeof(GameObject*));
    if (!scene->rootObjects) {
        free(scene->gameObjects);
        free(scene);
        return NULL;
    }
    memset(scene->rootObjects, 0, scene->rootObjectCapacity * sizeof(GameObject*));
    
    // Initialize GameObject pool
    PoolResult result = object_pool_init(&scene->gameObjectPool, 
                                        sizeof(GameObject), 
                                        maxGameObjects, 
                                        "SceneGameObjects");
    if (result != POOL_OK) {
        free(scene->rootObjects);
        free(scene->gameObjects);
        free(scene);
        return NULL;
    }
    
    // Initialize component pools (for basic component types)
    for (uint32_t i = 0; i < 32; i++) {
        ComponentType type = 1 << i;
        if (i < 8) { // Initialize pools for basic types
            object_pool_init(&scene->componentPools[i], 
                           64, // Basic component size estimation 
                           maxGameObjects / 2, // Assume not all objects have every component
                           "SceneComponent");
        }
    }
    
    // Allocate component arrays for batch processing
    scene->transformComponents = malloc(maxGameObjects * sizeof(Component*));
    scene->spriteComponents = malloc(maxGameObjects * sizeof(Component*));
    scene->collisionComponents = malloc(maxGameObjects * sizeof(Component*));
    
    if (!scene->transformComponents || !scene->spriteComponents || !scene->collisionComponents) {
        // Cleanup on failure
        if (scene->transformComponents) free(scene->transformComponents);
        if (scene->spriteComponents) free(scene->spriteComponents);
        if (scene->collisionComponents) free(scene->collisionComponents);
        object_pool_destroy(&scene->gameObjectPool);
        for (uint32_t i = 0; i < 8; i++) {
            object_pool_destroy(&scene->componentPools[i]);
        }
        free(scene->rootObjects);
        free(scene->gameObjects);
        free(scene);
        return NULL;
    }
    
    // Initialize component arrays to NULL
    memset(scene->transformComponents, 0, maxGameObjects * sizeof(Component*));
    memset(scene->spriteComponents, 0, maxGameObjects * sizeof(Component*));
    memset(scene->collisionComponents, 0, maxGameObjects * sizeof(Component*));
    
    return scene;
}

void scene_destroy(Scene* scene) {
    if (!scene) return;
    
    // Set state to unloading and call callback
    if (scene->state != SCENE_STATE_INACTIVE && scene->onUnload) {
        scene->onUnload(scene);
    }
    
    // Destroy all GameObjects (they will remove themselves from the scene)
    // We iterate backwards to avoid issues with array shifting during removal
    for (int i = (int)scene->gameObjectCount - 1; i >= 0; i--) {
        if (scene->gameObjects[i]) {
            game_object_destroy(scene->gameObjects[i]);
        }
    }
    
    // Destroy object pools
    object_pool_destroy(&scene->gameObjectPool);
    for (uint32_t i = 0; i < 32; i++) {
        if (i < 8) { // Only destroy initialized pools
            object_pool_destroy(&scene->componentPools[i]);
        }
    }
    
    // Free arrays
    free(scene->gameObjects);
    free(scene->rootObjects);
    free(scene->transformComponents);
    free(scene->spriteComponents);
    free(scene->collisionComponents);
    
    free(scene);
}

// Scene state management
SceneResult scene_set_state(Scene* scene, SceneState state) {
    if (!scene) {
        return SCENE_ERROR_NULL_POINTER;
    }
    
    SceneState oldState = scene->state;
    scene->state = state;
    
    // Call lifecycle callbacks
    switch (state) {
        case SCENE_STATE_LOADING:
            if (scene->onLoad) {
                scene->onLoad(scene);
            }
            break;
            
        case SCENE_STATE_ACTIVE:
            if (oldState != SCENE_STATE_PAUSED && scene->onActivate) {
                scene->onActivate(scene);
            }
            break;
            
        case SCENE_STATE_PAUSED:
            if (scene->onDeactivate) {
                scene->onDeactivate(scene);
            }
            break;
            
        case SCENE_STATE_UNLOADING:
            if (scene->onUnload) {
                scene->onUnload(scene);
            }
            break;
            
        case SCENE_STATE_INACTIVE:
        default:
            break;
    }
    
    return SCENE_OK;
}

SceneState scene_get_state(const Scene* scene) {
    return scene ? scene->state : SCENE_STATE_INACTIVE;
}

void scene_set_time_scale(Scene* scene, float timeScale) {
    if (scene) {
        scene->timeScale = timeScale;
    }
}

float scene_get_time_scale(const Scene* scene) {
    return scene ? scene->timeScale : 1.0f;
}

// GameObject management
SceneResult scene_add_game_object(Scene* scene, GameObject* gameObject) {
    if (!scene || !gameObject) {
        return SCENE_ERROR_NULL_POINTER;
    }
    
    if (scene->gameObjectCount >= scene->gameObjectCapacity) {
        return SCENE_ERROR_POOL_FULL;
    }
    
    // Add to main GameObject array
    scene->gameObjects[scene->gameObjectCount] = gameObject;
    scene->gameObjectCount++;
    
    // Add to root objects if it has no parent
    if (!gameObject->parent) {
        if (scene->rootObjectCount >= scene->rootObjectCapacity) {
            // GameObject is added but not tracked as root - not ideal but won't crash
        } else {
            scene->rootObjects[scene->rootObjectCount] = gameObject;
            scene->rootObjectCount++;
        }
    }
    
    // Add components to batch arrays
    if (gameObject->transform) {
        scene->transformComponents[scene->transformCount] = (Component*)gameObject->transform;
        scene->transformCount++;
    }
    
    // Add other components as they're attached
    for (uint32_t i = 0; i < gameObject->componentCount; i++) {
        Component* component = gameObject->components[i];
        if (component && component != (Component*)gameObject->transform) {
            if (component->type == COMPONENT_TYPE_SPRITE && scene->spriteCount < scene->gameObjectCapacity) {
                scene->spriteComponents[scene->spriteCount] = component;
                scene->spriteCount++;
            } else if (component->type == COMPONENT_TYPE_COLLISION && scene->collisionCount < scene->gameObjectCapacity) {
                scene->collisionComponents[scene->collisionCount] = component;
                scene->collisionCount++;
            }
        }
    }
    
    // Update active count
    if (game_object_is_active(gameObject)) {
        scene->activeObjectCount++;
    }
    
    return SCENE_OK;
}

SceneResult scene_remove_game_object(Scene* scene, GameObject* gameObject) {
    if (!scene || !gameObject) {
        return SCENE_ERROR_NULL_POINTER;
    }
    
    // Find and remove from main array
    bool found = false;
    for (uint32_t i = 0; i < scene->gameObjectCount; i++) {
        if (scene->gameObjects[i] == gameObject) {
            // Shift remaining objects down
            for (uint32_t j = i; j < scene->gameObjectCount - 1; j++) {
                scene->gameObjects[j] = scene->gameObjects[j + 1];
            }
            scene->gameObjectCount--;
            found = true;
            break;
        }
    }
    
    if (!found) {
        return SCENE_ERROR_OBJECT_NOT_FOUND;
    }
    
    // Remove from root objects if present
    for (uint32_t i = 0; i < scene->rootObjectCount; i++) {
        if (scene->rootObjects[i] == gameObject) {
            for (uint32_t j = i; j < scene->rootObjectCount - 1; j++) {
                scene->rootObjects[j] = scene->rootObjects[j + 1];
            }
            scene->rootObjectCount--;
            break;
        }
    }
    
    // Rebuild component arrays (this is inefficient but simple for now)
    scene_rebuild_component_arrays(scene);
    
    // Update active count
    if (game_object_is_active(gameObject)) {
        scene->activeObjectCount--;
    }
    
    return SCENE_OK;
}

GameObject* scene_find_game_object_by_id(Scene* scene, uint32_t id) {
    if (!scene || id == GAMEOBJECT_INVALID_ID) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < scene->gameObjectCount; i++) {
        if (scene->gameObjects[i] && game_object_get_id(scene->gameObjects[i]) == id) {
            return scene->gameObjects[i];
        }
    }
    
    return NULL;
}

uint32_t scene_get_game_object_count(const Scene* scene) {
    return scene ? scene->gameObjectCount : 0;
}

// Component system registration
SceneResult scene_register_component_system(Scene* scene, ComponentType type,
                                           void (*updateBatch)(Component**, uint32_t, float),
                                           void (*renderBatch)(Component**, uint32_t),
                                           uint32_t priority) {
    if (!scene) {
        return SCENE_ERROR_NULL_POINTER;
    }
    
    if (scene->systemCount >= 32) {
        return SCENE_ERROR_POOL_FULL;
    }
    
    // Find existing system or create new one
    ComponentSystem* system = NULL;
    for (uint32_t i = 0; i < scene->systemCount; i++) {
        if (scene->systems[i].type == type) {
            system = &scene->systems[i];
            break;
        }
    }
    
    if (!system) {
        // Create new system
        system = &scene->systems[scene->systemCount];
        scene->systemCount++;
    }
    
    system->type = type;
    system->updateBatch = updateBatch;
    system->renderBatch = renderBatch;
    system->enabled = true;
    system->priority = priority;
    
    return SCENE_OK;
}

SceneResult scene_enable_component_system(Scene* scene, ComponentType type, bool enabled) {
    if (!scene) {
        return SCENE_ERROR_NULL_POINTER;
    }
    
    // Find the system
    for (uint32_t i = 0; i < scene->systemCount; i++) {
        if (scene->systems[i].type == type) {
            scene->systems[i].enabled = enabled;
            return SCENE_OK;
        }
    }
    
    return SCENE_ERROR_SYSTEM_NOT_FOUND;
}

// Scene updates
void scene_update(Scene* scene, float deltaTime) {
    if (!scene || scene->state != SCENE_STATE_ACTIVE) {
        return;
    }
    
    clock_t start = clock();
    
    // Apply time scale
    float scaledDeltaTime = deltaTime * scene->timeScale;
    
    // Update scene time
    scene->totalTime += scaledDeltaTime;
    scene->frameCount++;
    
    // Run component systems in priority order
    for (uint32_t priority = 0; priority < 10; priority++) {
        for (uint32_t i = 0; i < scene->systemCount; i++) {
            ComponentSystem* system = &scene->systems[i];
            if (system->enabled && system->priority == priority && system->updateBatch) {
                
                // Get components of this type
                Component** components = NULL;
                uint32_t count = 0;
                
                switch (system->type) {
                    case COMPONENT_TYPE_TRANSFORM:
                        components = scene->transformComponents;
                        count = scene->transformCount;
                        break;
                    case COMPONENT_TYPE_SPRITE:
                        components = scene->spriteComponents;
                        count = scene->spriteCount;
                        break;
                    case COMPONENT_TYPE_COLLISION:
                        components = scene->collisionComponents;
                        count = scene->collisionCount;
                        break;
                    default:
                        break;
                }
                
                if (components && count > 0) {
                    system->updateBatch(components, count, scaledDeltaTime);
                }
            }
        }
    }
    
    clock_t end = clock();
    scene->lastUpdateTime = ((float)(end - start)) / CLOCKS_PER_SEC * 1000.0f; // milliseconds
}

void scene_fixed_update(Scene* scene, float fixedDeltaTime) {
    // For now, just call regular update - can be extended later for physics
    if (scene && scene->state == SCENE_STATE_ACTIVE) {
        scene_update(scene, fixedDeltaTime);
    }
}

void scene_render(Scene* scene) {
    if (!scene || scene->state != SCENE_STATE_ACTIVE) {
        return;
    }
    
    clock_t start = clock();
    
    // Run render systems
    for (uint32_t i = 0; i < scene->systemCount; i++) {
        ComponentSystem* system = &scene->systems[i];
        if (system->enabled && system->renderBatch) {
            
            Component** components = NULL;
            uint32_t count = 0;
            
            switch (system->type) {
                case COMPONENT_TYPE_SPRITE:
                    components = scene->spriteComponents;
                    count = scene->spriteCount;
                    break;
                default:
                    break;
            }
            
            if (components && count > 0) {
                system->renderBatch(components, count);
            }
        }
    }
    
    clock_t end = clock();
    scene->lastRenderTime = ((float)(end - start)) / CLOCKS_PER_SEC * 1000.0f; // milliseconds
}

// Batch operations
void scene_update_transforms(Scene* scene, float deltaTime) {
    // TODO: Implement transform batch update
}

void scene_update_sprites(Scene* scene, float deltaTime) {
    // TODO: Implement sprite batch update
}

void scene_render_sprites(Scene* scene) {
    // TODO: Implement sprite batch rendering
}

// Resource access
ObjectPool* scene_get_gameobject_pool(Scene* scene) {
    return scene ? &scene->gameObjectPool : NULL;
}

ObjectPool* scene_get_component_pool(Scene* scene, ComponentType type) {
    if (!scene) return NULL;
    
    // Find the bit position for the component type
    for (uint32_t i = 0; i < 32; i++) {
        if ((1 << i) == type) {
            return i < 8 ? &scene->componentPools[i] : NULL;
        }
    }
    return NULL;
}

// Debug and profiling
void scene_print_stats(const Scene* scene) {
    if (!scene) {
        printf("Scene stats: NULL scene\n");
        return;
    }
    
    printf("=== Scene '%s' Stats ===\n", scene->name);
    printf("State: %d\n", (int)scene->state);
    printf("GameObjects: %u / %u\n", scene->gameObjectCount, scene->gameObjectCapacity);
    printf("Active Objects: %u\n", scene->activeObjectCount);
    printf("Root Objects: %u / %u\n", scene->rootObjectCount, scene->rootObjectCapacity);
    printf("Components - Transform: %u, Sprite: %u, Collision: %u\n", 
           scene->transformCount, scene->spriteCount, scene->collisionCount);
    printf("Time Scale: %.2f\n", scene->timeScale);
    printf("Total Time: %.2f\n", scene->totalTime);
    printf("Frames: %u\n", scene->frameCount);
    printf("Last Update: %.3f ms\n", scene->lastUpdateTime);
    printf("Last Render: %.3f ms\n", scene->lastRenderTime);
    printf("========================\n");
}

uint32_t scene_get_memory_usage(const Scene* scene) {
    if (!scene) return 0;
    
    uint32_t usage = sizeof(Scene);
    usage += scene->gameObjectCapacity * sizeof(GameObject*); // gameObjects array
    usage += scene->rootObjectCapacity * sizeof(GameObject*); // rootObjects array
    usage += scene->gameObjectCapacity * sizeof(Component*) * 3; // component arrays
    
    // Add pool memory usage (estimate)
    usage += scene->gameObjectCapacity * sizeof(GameObject); // GameObject pool
    
    return usage;
}

// Helper function for rebuilding component arrays
void scene_rebuild_component_arrays(Scene* scene) {
    if (!scene) return;
    
    scene->transformCount = 0;
    scene->spriteCount = 0;
    scene->collisionCount = 0;
    
    for (uint32_t i = 0; i < scene->gameObjectCount; i++) {
        GameObject* gameObject = scene->gameObjects[i];
        if (!gameObject) continue;
        
        // Add transform
        if (gameObject->transform) {
            scene->transformComponents[scene->transformCount] = (Component*)gameObject->transform;
            scene->transformCount++;
        }
        
        // Add other components
        for (uint32_t j = 0; j < gameObject->componentCount; j++) {
            Component* component = gameObject->components[j];
            if (!component) continue;
            
            if (component->type == COMPONENT_TYPE_SPRITE && scene->spriteCount < scene->gameObjectCapacity) {
                scene->spriteComponents[scene->spriteCount] = component;
                scene->spriteCount++;
            } else if (component->type == COMPONENT_TYPE_COLLISION && scene->collisionCount < scene->gameObjectCapacity) {
                scene->collisionComponents[scene->collisionCount] = component;
                scene->collisionCount++;
            }
        }
    }
}