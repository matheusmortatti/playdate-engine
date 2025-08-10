#ifndef MOCK_SCENE_H
#define MOCK_SCENE_H

#include "../../src/core/game_object.h"
#include "../../src/core/memory_pool.h"
#include <stdint.h>

#define MAX_GAMEOBJECTS_IN_MOCK_SCENE 15000

// Mock Scene structure for testing GameObject without Phase 4 dependencies
typedef struct MockScene {
    ObjectPool gameObjectPool;
    GameObject** gameObjects; // Dynamic array
    uint32_t gameObjectCount;
    uint32_t maxGameObjects;
    const char* debugName;
} MockScene;

// Mock scene lifecycle
MockScene* mock_scene_create(void);
MockScene* mock_scene_create_with_capacity(uint32_t maxGameObjects, const char* debugName);
void mock_scene_destroy(MockScene* scene);

// Mock scene statistics (for testing)
uint32_t mock_scene_get_gameobject_count(MockScene* scene);
uint32_t mock_scene_get_max_gameobjects(MockScene* scene);
float mock_scene_get_usage_percent(MockScene* scene);

// Internal functions used by game_object.c
ObjectPool* scene_get_gameobject_pool(Scene* scene);
void scene_add_game_object(Scene* scene, GameObject* gameObject);
void scene_remove_game_object(Scene* scene, GameObject* gameObject);

#endif // MOCK_SCENE_H