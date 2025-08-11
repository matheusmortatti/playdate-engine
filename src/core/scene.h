#ifndef SCENE_H
#define SCENE_H

#include "game_object.h"
#include "memory_pool.h"
#include "component.h"
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define MAX_GAMEOBJECTS_PER_SCENE 10000
#define SCENE_INVALID_ID 0

// Forward declarations
typedef struct Scene Scene;
typedef struct SceneManager SceneManager;

// Scene state enumeration
typedef enum {
    SCENE_STATE_INACTIVE = 0,
    SCENE_STATE_LOADING,
    SCENE_STATE_ACTIVE,
    SCENE_STATE_PAUSED,
    SCENE_STATE_UNLOADING
} SceneState;

// Component system information
typedef struct ComponentSystem {
    ComponentType type;
    void (*updateBatch)(Component** components, uint32_t count, float deltaTime);
    void (*renderBatch)(Component** components, uint32_t count);
    bool enabled;
    uint32_t priority; // Lower numbers update first
} ComponentSystem;

// Scene structure
typedef struct Scene {
    uint32_t id;                              // Unique scene identifier
    char name[64];                            // Debug name
    SceneState state;                         // Current scene state
    
    // GameObject management
    GameObject** gameObjects;                 // Dynamic array of GameObjects
    uint32_t gameObjectCount;                 // Current number of GameObjects
    uint32_t gameObjectCapacity;              // Maximum GameObjects
    
    // Object pools for scene-local allocation
    ObjectPool gameObjectPool;                // Pool for GameObjects
    ObjectPool componentPools[32];            // Pools for each component type (max 32 types)
    
    // Component systems for batch processing
    ComponentSystem systems[32];              // Component systems (max 32 types)
    uint32_t systemCount;
    
    // Component arrays for efficient batch operations
    Component** transformComponents;          // All transform components
    Component** spriteComponents;             // All sprite components
    Component** collisionComponents;          // All collision components
    uint32_t transformCount;
    uint32_t spriteCount;
    uint32_t collisionCount;
    
    // Scene hierarchy root objects (objects with no parent)
    GameObject** rootObjects;
    uint32_t rootObjectCount;
    uint32_t rootObjectCapacity;
    
    // Timing and frame information
    float timeScale;                          // Time scaling factor
    float totalTime;                          // Total scene time
    uint32_t frameCount;                      // Frames since scene activation
    
    // Scene lifecycle callbacks
    void (*onLoad)(Scene* scene);
    void (*onUnload)(Scene* scene);
    void (*onActivate)(Scene* scene);
    void (*onDeactivate)(Scene* scene);
    
    // Performance statistics
    float lastUpdateTime;
    float lastRenderTime;
    uint32_t activeObjectCount;
    
} Scene;

// Scene management results
typedef enum {
    SCENE_OK = 0,
    SCENE_ERROR_NULL_POINTER,
    SCENE_ERROR_OUT_OF_MEMORY,
    SCENE_ERROR_OBJECT_NOT_FOUND,
    SCENE_ERROR_POOL_FULL,
    SCENE_ERROR_INVALID_STATE,
    SCENE_ERROR_SYSTEM_NOT_FOUND
} SceneResult;

// Scene lifecycle
Scene* scene_create(const char* name, uint32_t maxGameObjects);
void scene_destroy(Scene* scene);

// Scene state management
SceneResult scene_set_state(Scene* scene, SceneState state);
SceneState scene_get_state(const Scene* scene);
void scene_set_time_scale(Scene* scene, float timeScale);
float scene_get_time_scale(const Scene* scene);

// GameObject management
SceneResult scene_add_game_object(Scene* scene, GameObject* gameObject);
SceneResult scene_remove_game_object(Scene* scene, GameObject* gameObject);
GameObject* scene_find_game_object_by_id(Scene* scene, uint32_t id);
uint32_t scene_get_game_object_count(const Scene* scene);

// Component system registration
SceneResult scene_register_component_system(Scene* scene, ComponentType type,
                                           void (*updateBatch)(Component**, uint32_t, float),
                                           void (*renderBatch)(Component**, uint32_t),
                                           uint32_t priority);
SceneResult scene_enable_component_system(Scene* scene, ComponentType type, bool enabled);

// Scene updates (called by SceneManager)
void scene_update(Scene* scene, float deltaTime);
void scene_fixed_update(Scene* scene, float fixedDeltaTime);
void scene_render(Scene* scene);

// Batch operations
void scene_update_transforms(Scene* scene, float deltaTime);
void scene_update_sprites(Scene* scene, float deltaTime);
void scene_render_sprites(Scene* scene);

// Resource access
ObjectPool* scene_get_gameobject_pool(Scene* scene);
ObjectPool* scene_get_component_pool(Scene* scene, ComponentType type);

// Debug and profiling
void scene_print_stats(const Scene* scene);
uint32_t scene_get_memory_usage(const Scene* scene);

// Internal helper function (exposed for testing)
void scene_rebuild_component_arrays(Scene* scene);

// Fast access helpers
static inline uint32_t scene_get_active_object_count(const Scene* scene) {
    return scene ? scene->activeObjectCount : 0;
}

static inline bool scene_is_active(const Scene* scene) {
    return scene ? scene->state == SCENE_STATE_ACTIVE : false;
}

#endif // SCENE_H