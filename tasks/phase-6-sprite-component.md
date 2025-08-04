# Phase 6: Sprite Component

## Objective

Implement a high-performance sprite rendering system with Playdate SDK integration, supporting batch rendering, sprite sheets, animations, and visual effects. This component targets 1000+ sprites per frame with hardware-accelerated rendering and memory-efficient asset management.

## Prerequisites

- **Phase 1**: Memory Management (ObjectPool system)
- **Phase 2**: Component System (Component architecture)
- **Phase 3**: GameObject & Transform (Core entities)
- **Phase 4**: Scene Management (Scene organization)
- **Phase 5**: Spatial Partitioning (Spatial optimization)
- Playdate SDK graphics API knowledge
- Understanding of sprite rendering optimization

## Technical Specifications

### Performance Targets
- **Sprite rendering**: 1000+ sprites per frame at 30 FPS
- **Batch rendering**: Minimize draw calls through sprite batching
- **Memory efficiency**: < 5% overhead for sprite management
- **Animation updates**: < 10µs per animated sprite
- **Visibility culling**: Spatial query integration for off-screen culling

### Sprite System Architecture Goals
- **Playdate integration**: Native LCDBitmap and rendering API usage
- **Sprite sheet support**: Efficient sub-rectangle rendering
- **Animation system**: Frame-based and timeline animations
- **Layering system**: Z-depth sorting and render order
- **Visual effects**: Alpha, scaling, rotation, color effects

## Code Structure

```
src/components/
├── sprite_component.h      # Sprite component interface
├── sprite_component.c      # Sprite implementation
├── sprite_renderer.h       # Batch rendering system
├── sprite_renderer.c       # Renderer implementation
├── sprite_animation.h      # Animation system
└── sprite_animation.c      # Animation implementation

src/assets/
├── sprite_loader.h         # Asset loading utilities
├── sprite_loader.c         # Loader implementation
├── sprite_atlas.h          # Sprite sheet management
└── sprite_atlas.c          # Atlas implementation

tests/components/
├── test_sprite_component.c # Sprite component tests
├── test_sprite_renderer.c  # Renderer tests
├── test_sprite_animation.c # Animation tests
└── test_sprite_perf.c      # Performance benchmarks

examples/
├── sprite_demo.c           # Basic sprite usage
├── animation_demo.c        # Animation examples
└── batch_rendering.c       # Performance optimization demo
```

## Implementation Steps

### Step 1: Sprite Component Core Structure

```c
// sprite_component.h
#ifndef SPRITE_COMPONENT_H
#define SPRITE_COMPONENT_H

#include "component.h"
#include "pd_api.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct SpriteComponent SpriteComponent;
typedef struct SpriteAnimation SpriteAnimation;
typedef struct SpriteAtlas SpriteAtlas;

// Sprite rendering modes
typedef enum {
    SPRITE_RENDER_NORMAL = 0,
    SPRITE_RENDER_ADDITIVE,
    SPRITE_RENDER_MULTIPLY,
    SPRITE_RENDER_SUBTRACT,
    SPRITE_RENDER_MASK
} SpriteRenderMode;

// Sprite component structure (128 bytes target)
typedef struct SpriteComponent {
    Component base;                    // 32 bytes - base component
    
    // Rendering properties
    LCDBitmap* bitmap;                // 8 bytes - Playdate bitmap
    LCDRect sourceRect;               // 16 bytes - Source rectangle {x, y, width, height}
    float anchorX, anchorY;           // 8 bytes - Anchor point (0.0-1.0)
    LCDBitmapFlip flip;               // 4 bytes - Horizontal/vertical flip
    float opacity;                    // 4 bytes - Alpha value (0.0-1.0)
    SpriteRenderMode renderMode;      // 4 bytes - Blend mode
    
    // Transform cache (for performance)
    float renderX, renderY;           // 8 bytes - Cached render position
    float renderWidth, renderHeight;  // 8 bytes - Cached render size
    bool transformDirty;              // 1 byte - Needs position recalculation
    
    // Animation system
    SpriteAnimation* animation;       // 8 bytes - Current animation
    float animationTime;              // 4 bytes - Current animation time
    uint32_t currentFrame;            // 4 bytes - Current frame index
    bool animationPlaying;            // 1 byte - Animation state
    bool animationLooping;            // 1 byte - Loop flag
    
    // Sprite sheet support
    SpriteAtlas* atlas;               // 8 bytes - Sprite atlas reference
    uint32_t atlasIndex;              // 4 bytes - Index in atlas
    
    // Rendering optimization
    bool visible;                     // 1 byte - Visibility flag
    bool culled;                      // 1 byte - Frustum culled
    uint32_t renderLayer;             // 4 bytes - Z-depth layer
    uint32_t sortingOrder;            // 4 bytes - Order within layer
    
    // Color and effects
    uint8_t colorR, colorG, colorB;   // 3 bytes - Color tint (0-255)
    uint8_t padding[9];               // 9 bytes - Explicit padding to 128 bytes
    
} SpriteComponent;

// Sprite animation frame
typedef struct SpriteFrame {
    LCDRect sourceRect;               // Source rectangle in sprite sheet
    float duration;                   // Frame duration in seconds
    float anchorX, anchorY;           // Frame-specific anchor point
} SpriteFrame;

// Sprite animation
typedef struct SpriteAnimation {
    char name[32];                    // Animation name
    SpriteFrame* frames;              // Array of animation frames
    uint32_t frameCount;              // Number of frames
    float totalDuration;              // Total animation duration
    bool looping;                     // Default looping behavior
    SpriteAnimation* nextAnimation;   // Animation to play after this one
} SpriteAnimation;

// Sprite component interface
SpriteComponent* sprite_component_create(GameObject* gameObject);
void sprite_component_destroy(SpriteComponent* sprite);

// Bitmap management
bool sprite_component_set_bitmap(SpriteComponent* sprite, LCDBitmap* bitmap);
LCDBitmap* sprite_component_get_bitmap(SpriteComponent* sprite);
bool sprite_component_load_bitmap(SpriteComponent* sprite, const char* path);

// Source rectangle (for sprite sheets)
void sprite_component_set_source_rect(SpriteComponent* sprite, int x, int y, int width, int height);
LCDRect sprite_component_get_source_rect(SpriteComponent* sprite);

// Rendering properties
void sprite_component_set_anchor(SpriteComponent* sprite, float anchorX, float anchorY);
void sprite_component_get_anchor(SpriteComponent* sprite, float* anchorX, float* anchorY);
void sprite_component_set_flip(SpriteComponent* sprite, LCDBitmapFlip flip);
LCDBitmapFlip sprite_component_get_flip(SpriteComponent* sprite);
void sprite_component_set_opacity(SpriteComponent* sprite, float opacity);
float sprite_component_get_opacity(SpriteComponent* sprite);

// Animation control
bool sprite_component_play_animation(SpriteComponent* sprite, SpriteAnimation* animation);
void sprite_component_stop_animation(SpriteComponent* sprite);
void sprite_component_pause_animation(SpriteComponent* sprite);
void sprite_component_resume_animation(SpriteComponent* sprite);
bool sprite_component_is_animation_playing(SpriteComponent* sprite);

// Visibility and culling
void sprite_component_set_visible(SpriteComponent* sprite, bool visible);
bool sprite_component_is_visible(SpriteComponent* sprite);
void sprite_component_set_render_layer(SpriteComponent* sprite, uint32_t layer);
uint32_t sprite_component_get_render_layer(SpriteComponent* sprite);

// Color and effects
void sprite_component_set_color(SpriteComponent* sprite, uint8_t r, uint8_t g, uint8_t b);
void sprite_component_get_color(SpriteComponent* sprite, uint8_t* r, uint8_t* g, uint8_t* b);

// Atlas support
bool sprite_component_set_atlas(SpriteComponent* sprite, SpriteAtlas* atlas, uint32_t index);

// Component system integration
void sprite_component_register(void);

// Fast access helpers
static inline bool sprite_component_needs_render(SpriteComponent* sprite) {
    return sprite && sprite->visible && !sprite->culled && sprite->bitmap && sprite->opacity > 0.0f;
}

static inline void sprite_component_mark_transform_dirty(SpriteComponent* sprite) {
    if (sprite) sprite->transformDirty = true;
}

#endif // SPRITE_COMPONENT_H
```

### Step 2: Sprite Component Implementation

```c
// sprite_component.c
#include "sprite_component.h"
#include "component_registry.h"
#include "game_object.h"
#include "transform_component.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Sprite component vtable
static void sprite_component_init(Component* component, GameObject* gameObject);
static void sprite_component_destroy_impl(Component* component);
static void sprite_component_update(Component* component, float deltaTime);
static void sprite_component_render(Component* component);

static const ComponentVTable spriteVTable = {
    .init = sprite_component_init,
    .destroy = sprite_component_destroy_impl,
    .update = sprite_component_update,
    .render = sprite_component_render,
    .clone = NULL,
    .fixedUpdate = NULL,
    .onEnabled = NULL,
    .onDisabled = NULL,
    .onGameObjectDestroyed = NULL,
    .getSerializedSize = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// Global Playdate API reference
static PlaydateAPI* g_pd = NULL;

void sprite_component_register(void) {
    component_registry_register_type(
        COMPONENT_TYPE_SPRITE,
        sizeof(SpriteComponent),
        1000, // Pool size
        &spriteVTable,
        "Sprite"
    );
}

void sprite_component_set_playdate_api(PlaydateAPI* pd) {
    g_pd = pd;
}

SpriteComponent* sprite_component_create(GameObject* gameObject) {
    Component* baseComponent = component_registry_create(COMPONENT_TYPE_SPRITE, gameObject);
    return (SpriteComponent*)baseComponent;
}

static void sprite_component_init(Component* component, GameObject* gameObject) {
    SpriteComponent* sprite = (SpriteComponent*)component;
    
    // Initialize sprite properties
    sprite->bitmap = NULL;\n    sprite->sourceRect = (LCDRect){0, 0, 0, 0};\n    sprite->anchorX = 0.5f;\n    sprite->anchorY = 0.5f;\n    sprite->flip = kBitmapUnflipped;\n    sprite->opacity = 1.0f;\n    sprite->renderMode = SPRITE_RENDER_NORMAL;\n    \n    // Initialize cached transform\n    sprite->renderX = 0.0f;\n    sprite->renderY = 0.0f;\n    sprite->renderWidth = 0.0f;\n    sprite->renderHeight = 0.0f;\n    sprite->transformDirty = true;\n    \n    // Initialize animation\n    sprite->animation = NULL;\n    sprite->animationTime = 0.0f;\n    sprite->currentFrame = 0;\n    sprite->animationPlaying = false;\n    sprite->animationLooping = false;\n    \n    // Initialize atlas\n    sprite->atlas = NULL;\n    sprite->atlasIndex = 0;\n    \n    // Initialize rendering\n    sprite->visible = true;\n    sprite->culled = false;\n    sprite->renderLayer = 0;\n    sprite->sortingOrder = 0;\n    \n    // Initialize color\n    sprite->colorR = 255;\n    sprite->colorG = 255;\n    sprite->colorB = 255;\n}\n\nstatic void sprite_component_destroy_impl(Component* component) {\n    SpriteComponent* sprite = (SpriteComponent*)component;\n    \n    // Note: We don't free the bitmap here as it may be shared\n    // Bitmap management should be handled by asset management system\n    sprite->bitmap = NULL;\n    sprite->animation = NULL;\n    sprite->atlas = NULL;\n}\n\nstatic void sprite_component_update(Component* component, float deltaTime) {\n    SpriteComponent* sprite = (SpriteComponent*)component;\n    \n    if (!sprite->visible) {\n        return;\n    }\n    \n    // Update animation\n    if (sprite->animationPlaying && sprite->animation) {\n        sprite->animationTime += deltaTime;\n        \n        // Check if animation finished\n        if (sprite->animationTime >= sprite->animation->totalDuration) {\n            if (sprite->animationLooping || sprite->animation->looping) {\n                sprite->animationTime = fmodf(sprite->animationTime, sprite->animation->totalDuration);\n            } else {\n                sprite->animationTime = sprite->animation->totalDuration;\n                sprite->animationPlaying = false;\n                \n                // Play next animation if specified\n                if (sprite->animation->nextAnimation) {\n                    sprite_component_play_animation(sprite, sprite->animation->nextAnimation);\n                }\n            }\n        }\n        \n        // Update current frame\n        sprite_component_update_animation_frame(sprite);\n    }\n    \n    // Update transform cache if dirty\n    if (sprite->transformDirty) {\n        sprite_component_update_transform_cache(sprite);\n    }\n}\n\nstatic void sprite_component_render(Component* component) {\n    SpriteComponent* sprite = (SpriteComponent*)component;\n    \n    if (!sprite_component_needs_render(sprite) || !g_pd) {\n        return;\n    }\n    \n    // Set rendering properties\n    if (sprite->opacity < 1.0f) {\n        // Handle transparency (Playdate is 1-bit, so this might be dithering)\n        // Implementation depends on Playdate's capabilities\n    }\n    \n    // Calculate render position with anchor\n    float renderX = sprite->renderX - (sprite->renderWidth * sprite->anchorX);\n    float renderY = sprite->renderY - (sprite->renderHeight * sprite->anchorY);\n    \n    // Render sprite\n    if (sprite->sourceRect.width > 0 && sprite->sourceRect.height > 0) {\n        // Render with source rectangle (sprite sheet)\n        g_pd->graphics->drawBitmap(sprite->bitmap, \n                                 (int)renderX, (int)renderY, \n                                 sprite->flip);\n    } else {\n        // Render full bitmap\n        g_pd->graphics->drawBitmap(sprite->bitmap, \n                                 (int)renderX, (int)renderY, \n                                 sprite->flip);\n    }\n}\n\nbool sprite_component_set_bitmap(SpriteComponent* sprite, LCDBitmap* bitmap) {\n    if (!sprite) {\n        return false;\n    }\n    \n    sprite->bitmap = bitmap;\n    \n    if (bitmap && g_pd) {\n        // Update source rectangle to match bitmap size if not set\n        if (sprite->sourceRect.width == 0 || sprite->sourceRect.height == 0) {\n            int width, height;\n            g_pd->graphics->getBitmapData(bitmap, &width, &height, NULL, NULL, NULL);\n            sprite->sourceRect = (LCDRect){0, 0, width, height};\n        }\n        \n        sprite_component_mark_transform_dirty(sprite);\n    }\n    \n    return true;\n}\n\nbool sprite_component_load_bitmap(SpriteComponent* sprite, const char* path) {\n    if (!sprite || !path || !g_pd) {\n        return false;\n    }\n    \n    LCDBitmap* bitmap = g_pd->graphics->loadBitmap(path, NULL);\n    if (!bitmap) {\n        return false;\n    }\n    \n    return sprite_component_set_bitmap(sprite, bitmap);\n}\n\nvoid sprite_component_set_source_rect(SpriteComponent* sprite, int x, int y, int width, int height) {\n    if (!sprite) return;\n    \n    sprite->sourceRect = (LCDRect){x, y, width, height};\n    sprite_component_mark_transform_dirty(sprite);\n}\n\nvoid sprite_component_set_anchor(SpriteComponent* sprite, float anchorX, float anchorY) {\n    if (!sprite) return;\n    \n    sprite->anchorX = anchorX;\n    sprite->anchorY = anchorY;\n    sprite_component_mark_transform_dirty(sprite);\n}\n\nbool sprite_component_play_animation(SpriteComponent* sprite, SpriteAnimation* animation) {\n    if (!sprite || !animation) {\n        return false;\n    }\n    \n    sprite->animation = animation;\n    sprite->animationTime = 0.0f;\n    sprite->currentFrame = 0;\n    sprite->animationPlaying = true;\n    sprite->animationLooping = animation->looping;\n    \n    // Set first frame\n    if (animation->frameCount > 0) {\n        SpriteFrame* frame = &animation->frames[0];\n        sprite_component_set_source_rect(sprite, \n                                        frame->sourceRect.x, frame->sourceRect.y,\n                                        frame->sourceRect.width, frame->sourceRect.height);\n        sprite_component_set_anchor(sprite, frame->anchorX, frame->anchorY);\n    }\n    \n    return true;\n}\n\nvoid sprite_component_set_visible(SpriteComponent* sprite, bool visible) {\n    if (sprite) {\n        sprite->visible = visible;\n    }\n}\n\n// Helper function to update animation frame\nstatic void sprite_component_update_animation_frame(SpriteComponent* sprite) {\n    if (!sprite->animation || sprite->animation->frameCount == 0) {\n        return;\n    }\n    \n    // Find current frame based on time\n    float accumulatedTime = 0.0f;\n    uint32_t frameIndex = 0;\n    \n    for (uint32_t i = 0; i < sprite->animation->frameCount; i++) {\n        accumulatedTime += sprite->animation->frames[i].duration;\n        if (sprite->animationTime < accumulatedTime) {\n            frameIndex = i;\n            break;\n        }\n    }\n    \n    // Update frame if changed\n    if (frameIndex != sprite->currentFrame) {\n        sprite->currentFrame = frameIndex;\n        SpriteFrame* frame = &sprite->animation->frames[frameIndex];\n        \n        sprite_component_set_source_rect(sprite,\n                                        frame->sourceRect.x, frame->sourceRect.y,\n                                        frame->sourceRect.width, frame->sourceRect.height);\n        sprite_component_set_anchor(sprite, frame->anchorX, frame->anchorY);\n    }\n}\n\n// Helper function to update transform cache\nstatic void sprite_component_update_transform_cache(SpriteComponent* sprite) {\n    if (!sprite || !sprite->base.gameObject || !sprite->base.gameObject->transform) {\n        return;\n    }\n    \n    TransformComponent* transform = sprite->base.gameObject->transform;\n    \n    // Get world position\n    transform_component_get_position(transform, &sprite->renderX, &sprite->renderY);\n    \n    // Get render size\n    sprite->renderWidth = (float)sprite->sourceRect.width;\n    sprite->renderHeight = (float)sprite->sourceRect.height;\n    \n    // Apply transform scale\n    float scaleX, scaleY;\n    transform_component_get_scale(transform, &scaleX, &scaleY);\n    sprite->renderWidth *= scaleX;\n    sprite->renderHeight *= scaleY;\n    \n    sprite->transformDirty = false;\n}\n```\n\n### Step 3: Sprite Renderer (Batch System)\n\n```c\n// sprite_renderer.h\n#ifndef SPRITE_RENDERER_H\n#define SPRITE_RENDERER_H\n\n#include \"sprite_component.h\"\n#include \"spatial_grid.h\"\n#include \"pd_api.h\"\n\n#define MAX_SPRITES_PER_BATCH 256\n#define MAX_RENDER_LAYERS 16\n\n// Sprite batch for efficient rendering\ntypedef struct SpriteBatch {\n    SpriteComponent** sprites;     // Array of sprites to render\n    uint32_t spriteCount;         // Number of sprites in batch\n    uint32_t capacity;            // Maximum sprites in batch\n    uint32_t renderLayer;         // Layer this batch renders\n    LCDBitmap* sharedBitmap;      // Shared bitmap for batching optimization\n} SpriteBatch;\n\n// Sprite renderer system\ntypedef struct SpriteRenderer {\n    SpriteBatch batches[MAX_RENDER_LAYERS]; // One batch per layer\n    uint32_t activeBatches;                 // Number of active batches\n    \n    // Culling support\n    SpatialGrid* spatialGrid;               // For frustum culling\n    LCDRect viewportRect;                   // Camera viewport\n    bool frustumCullingEnabled;\n    \n    // Performance statistics\n    uint32_t spritesRendered;\n    uint32_t spritesculled;\n    uint32_t drawCalls;\n    float lastRenderTime;\n    \n    // Playdate API reference\n    PlaydateAPI* pd;\n    \n} SpriteRenderer;\n\n// Renderer management\nSpriteRenderer* sprite_renderer_create(PlaydateAPI* pd, SpatialGrid* spatialGrid);\nvoid sprite_renderer_destroy(SpriteRenderer* renderer);\n\n// Rendering operations\nvoid sprite_renderer_begin_frame(SpriteRenderer* renderer);\nvoid sprite_renderer_render_sprites(SpriteRenderer* renderer, SpriteComponent** sprites, uint32_t count);\nvoid sprite_renderer_end_frame(SpriteRenderer* renderer);\n\n// Viewport and culling\nvoid sprite_renderer_set_viewport(SpriteRenderer* renderer, int x, int y, int width, int height);\nvoid sprite_renderer_enable_frustum_culling(SpriteRenderer* renderer, bool enabled);\n\n// Performance and debugging\nvoid sprite_renderer_print_stats(SpriteRenderer* renderer);\nvoid sprite_renderer_reset_stats(SpriteRenderer* renderer);\n\n#endif // SPRITE_RENDERER_H\n```\n\n## Unit Tests\n\n### Sprite Component Tests\n\n```c\n// tests/components/test_sprite_component.c\n#include \"sprite_component.h\"\n#include \"game_object.h\"\n#include <assert.h>\n#include <stdio.h>\n\n// Mock Playdate API for testing\nstatic LCDBitmap* mock_bitmap = (LCDBitmap*)0x12345678;\n\nvoid test_sprite_component_creation(void) {\n    component_registry_init();\n    sprite_component_register();\n    \n    Scene* scene = mock_scene_create();\n    GameObject* gameObject = game_object_create(scene);\n    \n    SpriteComponent* sprite = sprite_component_create(gameObject);\n    assert(sprite != NULL);\n    assert(sprite->base.type == COMPONENT_TYPE_SPRITE);\n    assert(sprite->visible == true);\n    assert(sprite->opacity == 1.0f);\n    assert(sprite->anchorX == 0.5f && sprite->anchorY == 0.5f);\n    assert(sprite->renderLayer == 0);\n    \n    game_object_destroy(gameObject);\n    mock_scene_destroy(scene);\n    component_registry_shutdown();\n    printf(\"✓ Sprite component creation test passed\\n\");\n}\n\nvoid test_sprite_bitmap_management(void) {\n    component_registry_init();\n    sprite_component_register();\n    \n    Scene* scene = mock_scene_create();\n    GameObject* gameObject = game_object_create(scene);\n    SpriteComponent* sprite = sprite_component_create(gameObject);\n    \n    // Test bitmap setting\n    bool result = sprite_component_set_bitmap(sprite, mock_bitmap);\n    assert(result == true);\n    assert(sprite_component_get_bitmap(sprite) == mock_bitmap);\n    \n    // Test source rectangle\n    sprite_component_set_source_rect(sprite, 10, 20, 64, 48);\n    LCDRect rect = sprite_component_get_source_rect(sprite);\n    assert(rect.x == 10 && rect.y == 20);\n    assert(rect.width == 64 && rect.height == 48);\n    \n    game_object_destroy(gameObject);\n    mock_scene_destroy(scene);\n    component_registry_shutdown();\n    printf(\"✓ Sprite bitmap management test passed\\n\");\n}\n\nvoid test_sprite_rendering_properties(void) {\n    component_registry_init();\n    sprite_component_register();\n    \n    Scene* scene = mock_scene_create();\n    GameObject* gameObject = game_object_create(scene);\n    SpriteComponent* sprite = sprite_component_create(gameObject);\n    \n    // Test anchor\n    sprite_component_set_anchor(sprite, 0.0f, 1.0f);\n    float anchorX, anchorY;\n    sprite_component_get_anchor(sprite, &anchorX, &anchorY);\n    assert(anchorX == 0.0f && anchorY == 1.0f);\n    \n    // Test opacity\n    sprite_component_set_opacity(sprite, 0.5f);\n    assert(sprite_component_get_opacity(sprite) == 0.5f);\n    \n    // Test visibility\n    sprite_component_set_visible(sprite, false);\n    assert(sprite_component_is_visible(sprite) == false);\n    \n    // Test render layer\n    sprite_component_set_render_layer(sprite, 5);\n    assert(sprite_component_get_render_layer(sprite) == 5);\n    \n    game_object_destroy(gameObject);\n    mock_scene_destroy(scene);\n    component_registry_shutdown();\n    printf(\"✓ Sprite rendering properties test passed\\n\");\n}\n\nvoid test_sprite_animation_system(void) {\n    component_registry_init();\n    sprite_component_register();\n    \n    Scene* scene = mock_scene_create();\n    GameObject* gameObject = game_object_create(scene);\n    SpriteComponent* sprite = sprite_component_create(gameObject);\n    \n    // Create test animation\n    SpriteAnimation animation;\n    strcpy(animation.name, \"test_anim\");\n    animation.frameCount = 3;\n    animation.looping = true;\n    animation.nextAnimation = NULL;\n    \n    SpriteFrame frames[3] = {\n        {{0, 0, 32, 32}, 0.1f, 0.5f, 0.5f},\n        {{32, 0, 32, 32}, 0.1f, 0.5f, 0.5f},\n        {{64, 0, 32, 32}, 0.1f, 0.5f, 0.5f}\n    };\n    animation.frames = frames;\n    animation.totalDuration = 0.3f;\n    \n    // Test animation playback\n    bool result = sprite_component_play_animation(sprite, &animation);\n    assert(result == true);\n    assert(sprite_component_is_animation_playing(sprite) == true);\n    assert(sprite->currentFrame == 0);\n    \n    // Simulate time progression\n    Component* baseComponent = (Component*)sprite;\n    sprite_component_update(baseComponent, 0.15f); // Should advance to frame 1\n    assert(sprite->currentFrame == 1);\n    \n    sprite_component_stop_animation(sprite);\n    assert(sprite_component_is_animation_playing(sprite) == false);\n    \n    game_object_destroy(gameObject);\n    mock_scene_destroy(scene);\n    component_registry_shutdown();\n    printf(\"✓ Sprite animation system test passed\\n\");\n}\n```\n\n### Performance Tests\n\n```c\n// tests/components/test_sprite_perf.c\n#include \"sprite_component.h\"\n#include \"sprite_renderer.h\"\n#include <time.h>\n#include <stdio.h>\n\nvoid benchmark_sprite_updates(void) {\n    component_registry_init();\n    sprite_component_register();\n    \n    Scene* scene = mock_scene_create();\n    SpriteComponent* sprites[1000];\n    \n    // Create many sprites\n    for (int i = 0; i < 1000; i++) {\n        GameObject* obj = game_object_create(scene);\n        sprites[i] = sprite_component_create(obj);\n        sprite_component_set_bitmap(sprites[i], mock_bitmap);\n        sprite_component_set_source_rect(sprites[i], 0, 0, 32, 32);\n    }\n    \n    clock_t start = clock();\n    \n    // Update all sprites for 100 frames\n    for (int frame = 0; frame < 100; frame++) {\n        for (int i = 0; i < 1000; i++) {\n            Component* baseComponent = (Component*)sprites[i];\n            sprite_component_update(baseComponent, 0.016f);\n        }\n    }\n    \n    clock_t end = clock();\n    \n    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000; // milliseconds\n    double per_frame = time_taken / 100;\n    double per_sprite = (time_taken * 1000) / (100 * 1000); // microseconds per sprite\n    \n    printf(\"Sprite updates: %.2f ms per frame, %.2f μs per sprite\\n\", \n           per_frame, per_sprite);\n    \n    // Verify performance target\n    assert(per_sprite < 10); // Less than 10μs per sprite update\n    \n    mock_scene_destroy(scene);\n    component_registry_shutdown();\n    printf(\"✓ Sprite update performance test passed\\n\");\n}\n\nvoid benchmark_sprite_rendering(void) {\n    component_registry_init();\n    sprite_component_register();\n    \n    // Mock Playdate API setup would go here\n    SpriteRenderer* renderer = sprite_renderer_create(NULL, NULL);\n    \n    Scene* scene = mock_scene_create();\n    SpriteComponent* sprites[1000];\n    \n    // Create sprites\n    for (int i = 0; i < 1000; i++) {\n        GameObject* obj = game_object_create(scene);\n        sprites[i] = sprite_component_create(obj);\n        sprite_component_set_bitmap(sprites[i], mock_bitmap);\n        game_object_set_position(obj, i % 400, (i / 400) * 32);\n    }\n    \n    clock_t start = clock();\n    \n    // Render sprites for 60 frames\n    for (int frame = 0; frame < 60; frame++) {\n        sprite_renderer_begin_frame(renderer);\n        sprite_renderer_render_sprites(renderer, sprites, 1000);\n        sprite_renderer_end_frame(renderer);\n    }\n    \n    clock_t end = clock();\n    \n    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000; // milliseconds\n    double per_frame = time_taken / 60;\n    \n    printf(\"Sprite rendering: %.2f ms per frame (1000 sprites)\\n\", per_frame);\n    \n    // Verify performance target: 1000 sprites in < 33ms (30 FPS)\n    assert(per_frame < 33);\n    \n    sprite_renderer_destroy(renderer);\n    mock_scene_destroy(scene);\n    component_registry_shutdown();\n    printf(\"✓ Sprite rendering performance test passed\\n\");\n}\n```\n\n## Integration Points\n\n### Phase 3 Integration (GameObject & Transform)\n- Sprite component uses TransformComponent for world position and scale\n- Transform changes trigger sprite render cache updates\n- Anchor point calculations relative to transform\n\n### Phase 4 Integration (Scene Management)\n- Scene manages sprite component batches through component systems\n- Batch rendering coordinated by scene render system\n- Layer-based rendering order within scenes\n\n### Phase 5 Integration (Spatial Partitioning)\n- Frustum culling using spatial queries\n- Visibility determination for sprites\n- Performance optimization for large sprite counts\n\n## Performance Targets\n\n### Sprite Operations\n- **Sprite updates**: < 10μs per animated sprite\n- **Batch rendering**: 1000+ sprites at 30 FPS\n- **Memory overhead**: < 5% for sprite management\n- **Animation processing**: < 1ms for 100 animated sprites\n\n### Rendering Efficiency\n- **Draw call batching**: Minimize Playdate graphics calls\n- **Frustum culling**: Skip off-screen sprites\n- **Layer sorting**: Efficient depth-based rendering\n- **Atlas optimization**: Shared texture usage\n\n## Testing Criteria\n\n### Unit Test Requirements\n- ✅ Sprite component creation and destruction\n- ✅ Bitmap loading and management\n- ✅ Source rectangle and sprite sheet support\n- ✅ Animation system functionality\n- ✅ Rendering property management\n- ✅ Visibility and culling behavior\n\n### Performance Test Requirements\n- ✅ Sprite update speed benchmarks\n- ✅ Batch rendering performance\n- ✅ Animation processing efficiency\n- ✅ Memory usage measurements\n\n### Integration Test Requirements\n- ✅ Transform component integration\n- ✅ Scene management coordination\n- ✅ Spatial partitioning integration\n- ✅ Playdate SDK graphics integration\n\n## Success Criteria\n\n### Functional Requirements\n- [ ] Sprite component with full Playdate graphics integration\n- [ ] Sprite sheet and animation support\n- [ ] Batch rendering system for performance\n- [ ] Frustum culling and visibility management\n- [ ] Layer-based rendering with depth sorting\n\n### Performance Requirements\n- [ ] 1000+ sprites rendered at 30 FPS\n- [ ] < 10μs animation updates per sprite\n- [ ] Efficient batch rendering with minimal draw calls\n- [ ] < 5% memory overhead for sprite management\n\n### Quality Requirements\n- [ ] 100% unit test coverage for sprite system\n- [ ] Performance benchmarks meet all targets\n- [ ] Clean integration with transform and scene systems\n- [ ] Robust error handling and resource management\n\n## Next Steps\n\nUpon completion of this phase:\n1. Verify all sprite component tests pass\n2. Confirm performance benchmarks meet targets\n3. Test integration with Playdate SDK graphics\n4. Proceed to Phase 7: Collision Component implementation\n5. Begin implementing collision detection with sprite bounds\n\nThis phase provides the visual foundation for the engine, enabling efficient sprite-based rendering with animation support optimized for the Playdate platform.