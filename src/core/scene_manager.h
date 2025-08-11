#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene.h"

#define MAX_SCENES 16

typedef struct SceneManager {
    Scene* scenes[MAX_SCENES];
    uint32_t sceneCount;
    Scene* activeScene;
    Scene* loadingScene;
    
    // Global timing
    float globalTimeScale;
    float fixedTimeStep;
    float accumulatedTime;
    
} SceneManager;

// Scene manager lifecycle
SceneManager* scene_manager_create(void);
void scene_manager_destroy(SceneManager* manager);

// Scene management
SceneResult scene_manager_add_scene(SceneManager* manager, Scene* scene);
SceneResult scene_manager_remove_scene(SceneManager* manager, Scene* scene);
Scene* scene_manager_find_scene(SceneManager* manager, const char* name);

// Scene activation
SceneResult scene_manager_set_active_scene(SceneManager* manager, Scene* scene);
Scene* scene_manager_get_active_scene(SceneManager* manager);

// Main update loop
void scene_manager_update(SceneManager* manager, float deltaTime);
void scene_manager_render(SceneManager* manager);

// Global settings
void scene_manager_set_time_scale(SceneManager* manager, float timeScale);
void scene_manager_set_fixed_timestep(SceneManager* manager, float fixedTimeStep);

#endif // SCENE_MANAGER_H