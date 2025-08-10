#include "mock_scene.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

MockScene* mock_scene_create(void) {
    return mock_scene_create_with_capacity(100, "MockScene");
}

MockScene* mock_scene_create_with_capacity(uint32_t maxGameObjects, const char* debugName) {
    MockScene* scene = malloc(sizeof(MockScene));
    if (!scene) {
        return NULL;
    }
    
    // Initialize mock scene
    memset(scene, 0, sizeof(MockScene));
    scene->maxGameObjects = maxGameObjects;
    scene->debugName = debugName;
    scene->gameObjectCount = 0;
    
    // Allocate dynamic array for GameObjects
    scene->gameObjects = malloc(maxGameObjects * sizeof(GameObject*));
    if (!scene->gameObjects) {
        free(scene);
        return NULL;
    }
    memset(scene->gameObjects, 0, maxGameObjects * sizeof(GameObject*));
    
    // Initialize GameObject pool
    PoolResult result = object_pool_init(&scene->gameObjectPool, 
                                       sizeof(GameObject), 
                                       scene->maxGameObjects, 
                                       "MockSceneGameObjects");
    if (result != POOL_OK) {
        free(scene->gameObjects);
        free(scene);
        return NULL;
    }
    
    return scene;
}

void mock_scene_destroy(MockScene* scene) {
    if (!scene) return;
    
    // Destroy object pool
    object_pool_destroy(&scene->gameObjectPool);
    
    // Free dynamic array
    if (scene->gameObjects) {
        free(scene->gameObjects);
    }
    
    // Free the scene
    free(scene);
}

uint32_t mock_scene_get_gameobject_count(MockScene* scene) {
    return scene ? scene->gameObjectCount : 0;
}

uint32_t mock_scene_get_max_gameobjects(MockScene* scene) {
    return scene ? scene->maxGameObjects : 0;
}

float mock_scene_get_usage_percent(MockScene* scene) {
    if (!scene || scene->maxGameObjects == 0) return 0.0f;
    return (float)scene->gameObjectCount / (float)scene->maxGameObjects * 100.0f;
}

// Implementation of functions expected by game_object.c
ObjectPool* scene_get_gameobject_pool(Scene* scene) {
    MockScene* mockScene = (MockScene*)scene;
    return &mockScene->gameObjectPool;
}

void scene_add_game_object(Scene* scene, GameObject* gameObject) {
    MockScene* mockScene = (MockScene*)scene;
    if (!mockScene || !gameObject) return;
    
    // Add to scene's GameObject list
    if (mockScene->gameObjectCount < mockScene->maxGameObjects) {
        mockScene->gameObjects[mockScene->gameObjectCount] = gameObject;
        mockScene->gameObjectCount++;
    }
}

void scene_remove_game_object(Scene* scene, GameObject* gameObject) {
    MockScene* mockScene = (MockScene*)scene;
    if (!mockScene || !gameObject) return;
    
    // Remove from scene's GameObject list
    for (uint32_t i = 0; i < mockScene->gameObjectCount; i++) {
        if (mockScene->gameObjects[i] == gameObject) {
            // Shift remaining objects
            for (uint32_t j = i; j < mockScene->gameObjectCount - 1; j++) {
                mockScene->gameObjects[j] = mockScene->gameObjects[j + 1];
            }
            mockScene->gameObjects[mockScene->gameObjectCount - 1] = NULL;
            mockScene->gameObjectCount--;
            break;
        }
    }
}