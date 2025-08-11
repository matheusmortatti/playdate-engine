#include "scene_manager.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Scene manager lifecycle
SceneManager* scene_manager_create(void) {
    SceneManager* manager = malloc(sizeof(SceneManager));
    if (!manager) {
        return NULL;
    }
    
    memset(manager, 0, sizeof(SceneManager));
    
    // Initialize default values
    manager->globalTimeScale = 1.0f;
    manager->fixedTimeStep = 1.0f / 60.0f; // 60 FPS default
    manager->accumulatedTime = 0.0f;
    
    return manager;
}

void scene_manager_destroy(SceneManager* manager) {
    if (!manager) return;
    
    // Deactivate current scene
    if (manager->activeScene) {
        scene_set_state(manager->activeScene, SCENE_STATE_INACTIVE);
    }
    
    // Destroy all managed scenes
    for (uint32_t i = 0; i < manager->sceneCount; i++) {
        if (manager->scenes[i]) {
            scene_destroy(manager->scenes[i]);
        }
    }
    
    free(manager);
}

// Scene management
SceneResult scene_manager_add_scene(SceneManager* manager, Scene* scene) {
    if (!manager || !scene) {
        return SCENE_ERROR_NULL_POINTER;
    }
    
    if (manager->sceneCount >= MAX_SCENES) {
        return SCENE_ERROR_POOL_FULL;
    }
    
    // Check if scene already exists
    for (uint32_t i = 0; i < manager->sceneCount; i++) {
        if (manager->scenes[i] == scene) {
            return SCENE_ERROR_INVALID_STATE; // Already added
        }
    }
    
    // Add scene to manager
    manager->scenes[manager->sceneCount] = scene;
    manager->sceneCount++;
    
    return SCENE_OK;
}

SceneResult scene_manager_remove_scene(SceneManager* manager, Scene* scene) {
    if (!manager || !scene) {
        return SCENE_ERROR_NULL_POINTER;
    }
    
    // Find the scene
    uint32_t sceneIndex = MAX_SCENES; // Invalid index
    for (uint32_t i = 0; i < manager->sceneCount; i++) {
        if (manager->scenes[i] == scene) {
            sceneIndex = i;
            break;
        }
    }
    
    if (sceneIndex == MAX_SCENES) {
        return SCENE_ERROR_OBJECT_NOT_FOUND;
    }
    
    // If this is the active scene, deactivate it
    if (manager->activeScene == scene) {
        scene_set_state(scene, SCENE_STATE_INACTIVE);
        manager->activeScene = NULL;
    }
    
    // If this is the loading scene, clear it
    if (manager->loadingScene == scene) {
        manager->loadingScene = NULL;
    }
    
    // Remove from array by shifting remaining scenes
    for (uint32_t i = sceneIndex; i < manager->sceneCount - 1; i++) {
        manager->scenes[i] = manager->scenes[i + 1];
    }
    manager->sceneCount--;
    manager->scenes[manager->sceneCount] = NULL;
    
    return SCENE_OK;
}

Scene* scene_manager_find_scene(SceneManager* manager, const char* name) {
    if (!manager || !name) {
        return NULL;
    }
    
    // Search through all scenes
    for (uint32_t i = 0; i < manager->sceneCount; i++) {
        Scene* scene = manager->scenes[i];
        if (scene && strcmp(scene->name, name) == 0) {
            return scene;
        }
    }
    
    return NULL;
}

// Scene activation
SceneResult scene_manager_set_active_scene(SceneManager* manager, Scene* scene) {
    if (!manager) {
        return SCENE_ERROR_NULL_POINTER;
    }
    
    // Allow NULL scene (deactivate current)
    if (!scene) {
        if (manager->activeScene) {
            scene_set_state(manager->activeScene, SCENE_STATE_INACTIVE);
            manager->activeScene = NULL;
        }
        return SCENE_OK;
    }
    
    // Verify scene is managed by this manager
    bool found = false;
    for (uint32_t i = 0; i < manager->sceneCount; i++) {
        if (manager->scenes[i] == scene) {
            found = true;
            break;
        }
    }
    
    if (!found) {
        return SCENE_ERROR_OBJECT_NOT_FOUND;
    }
    
    // Deactivate current scene
    if (manager->activeScene && manager->activeScene != scene) {
        scene_set_state(manager->activeScene, SCENE_STATE_INACTIVE);
    }
    
    // Activate new scene
    manager->activeScene = scene;
    SceneResult result = scene_set_state(scene, SCENE_STATE_ACTIVE);
    
    if (result != SCENE_OK) {
        manager->activeScene = NULL; // Roll back on failure
        return result;
    }
    
    return SCENE_OK;
}

Scene* scene_manager_get_active_scene(SceneManager* manager) {
    return manager ? manager->activeScene : NULL;
}

// Main update loop
void scene_manager_update(SceneManager* manager, float deltaTime) {
    if (!manager) return;
    
    // Apply global time scale
    float scaledDeltaTime = deltaTime * manager->globalTimeScale;
    
    // Fixed timestep accumulation
    manager->accumulatedTime += scaledDeltaTime;
    
    // Process fixed updates while we have enough accumulated time
    while (manager->accumulatedTime >= manager->fixedTimeStep) {
        if (manager->activeScene) {
            scene_fixed_update(manager->activeScene, manager->fixedTimeStep);
        }
        manager->accumulatedTime -= manager->fixedTimeStep;
    }
    
    // Variable timestep update
    if (manager->activeScene) {
        scene_update(manager->activeScene, scaledDeltaTime);
    }
    
    // Handle scene loading/transition if needed
    if (manager->loadingScene) {
        SceneState loadingState = scene_get_state(manager->loadingScene);
        if (loadingState == SCENE_STATE_LOADING) {
            // Scene finished loading, activate it
            scene_manager_set_active_scene(manager, manager->loadingScene);
            manager->loadingScene = NULL;
        }
    }
}

void scene_manager_render(SceneManager* manager) {
    if (!manager) return;
    
    // Render active scene
    if (manager->activeScene) {
        scene_render(manager->activeScene);
    }
}

// Global settings
void scene_manager_set_time_scale(SceneManager* manager, float timeScale) {
    if (manager) {
        manager->globalTimeScale = timeScale;
    }
}

void scene_manager_set_fixed_timestep(SceneManager* manager, float fixedTimeStep) {
    if (manager && fixedTimeStep > 0.0f) {
        manager->fixedTimeStep = fixedTimeStep;
        // Reset accumulated time to prevent large jumps
        manager->accumulatedTime = 0.0f;
    }
}