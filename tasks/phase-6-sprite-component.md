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
    sprite->bitmap = NULL;
    sprite->sourceRect = (LCDRect){0, 0, 0, 0};
    sprite->anchorX = 0.5f;
    sprite->anchorY = 0.5f;
    sprite->flip = kBitmapUnflipped;
    sprite->opacity = 1.0f;
    sprite->renderMode = SPRITE_RENDER_NORMAL;
    
    // Initialize cached transform
    sprite->renderX = 0.0f;
    sprite->renderY = 0.0f;
    sprite->renderWidth = 0.0f;
    sprite->renderHeight = 0.0f;
    sprite->transformDirty = true;
    
    // Initialize animation
    sprite->animation = NULL;
    sprite->animationTime = 0.0f;
    sprite->currentFrame = 0;
    sprite->animationPlaying = false;
    sprite->animationLooping = false;
    
    // Initialize atlas
    sprite->atlas = NULL;
    sprite->atlasIndex = 0;
    
    // Initialize rendering
    sprite->visible = true;
    sprite->culled = false;
    sprite->renderLayer = 0;
    sprite->sortingOrder = 0;
    
    // Initialize color
    sprite->colorR = 255;
    sprite->colorG = 255;
    sprite->colorB = 255;
}

static void sprite_component_destroy_impl(Component* component) {
    SpriteComponent* sprite = (SpriteComponent*)component;
    
    // Note: We don't free the bitmap here as it may be shared
    // Bitmap management should be handled by asset management system
    sprite->bitmap = NULL;
    sprite->animation = NULL;
    sprite->atlas = NULL;
}

static void sprite_component_update(Component* component, float deltaTime) {
    SpriteComponent* sprite = (SpriteComponent*)component;
    
    if (!sprite->visible) {
        return;
    }
    
    // Update animation
    if (sprite->animationPlaying && sprite->animation) {
        sprite->animationTime += deltaTime;
        
        // Check if animation finished
        if (sprite->animationTime >= sprite->animation->totalDuration) {
            if (sprite->animationLooping || sprite->animation->looping) {
                sprite->animationTime = fmodf(sprite->animationTime, sprite->animation->totalDuration);
            } else {
                sprite->animationTime = sprite->animation->totalDuration;
                sprite->animationPlaying = false;
                
                // Play next animation if specified
                if (sprite->animation->nextAnimation) {
                    sprite_component_play_animation(sprite, sprite->animation->nextAnimation);
                }
            }
        }
        
        // Update current frame
        sprite_component_update_animation_frame(sprite);
    }
    
    // Update transform cache if dirty
    if (sprite->transformDirty) {
        sprite_component_update_transform_cache(sprite);
    }
}

static void sprite_component_render(Component* component) {
    SpriteComponent* sprite = (SpriteComponent*)component;
    
    if (!sprite_component_needs_render(sprite) || !g_pd) {
        return;
    }
    
    // Set rendering properties
    if (sprite->opacity < 1.0f) {
        // Handle transparency (Playdate is 1-bit, so this might be dithering)
        // Implementation depends on Playdate's capabilities
    }
    
    // Calculate render position with anchor
    float renderX = sprite->renderX - (sprite->renderWidth * sprite->anchorX);
    float renderY = sprite->renderY - (sprite->renderHeight * sprite->anchorY);
    
    // Render sprite
    if (sprite->sourceRect.width > 0 && sprite->sourceRect.height > 0) {
        // Render with source rectangle (sprite sheet)
        g_pd->graphics->drawBitmap(sprite->bitmap, 
                                 (int)renderX, (int)renderY, 
                                 sprite->flip);
    } else {
        // Render full bitmap
        g_pd->graphics->drawBitmap(sprite->bitmap, 
                                 (int)renderX, (int)renderY, 
                                 sprite->flip);
    }
}

bool sprite_component_set_bitmap(SpriteComponent* sprite, LCDBitmap* bitmap) {
    if (!sprite) {
        return false;
    }
    
    sprite->bitmap = bitmap;
    
    if (bitmap && g_pd) {
        // Update source rectangle to match bitmap size if not set
        if (sprite->sourceRect.width == 0 || sprite->sourceRect.height == 0) {
            int width, height;
            g_pd->graphics->getBitmapData(bitmap, &width, &height, NULL, NULL, NULL);
            sprite->sourceRect = (LCDRect){0, 0, width, height};
        }
        
        sprite_component_mark_transform_dirty(sprite);
    }
    
    return true;
}

bool sprite_component_load_bitmap(SpriteComponent* sprite, const char* path) {
    if (!sprite || !path || !g_pd) {
        return false;
    }
    
    LCDBitmap* bitmap = g_pd->graphics->loadBitmap(path, NULL);
    if (!bitmap) {
        return false;
    }
    
    return sprite_component_set_bitmap(sprite, bitmap);
}

void sprite_component_set_source_rect(SpriteComponent* sprite, int x, int y, int width, int height) {
    if (!sprite) return;
    
    sprite->sourceRect = (LCDRect){x, y, width, height};
    sprite_component_mark_transform_dirty(sprite);
}

void sprite_component_set_anchor(SpriteComponent* sprite, float anchorX, float anchorY) {
    if (!sprite) return;
    
    sprite->anchorX = anchorX;
    sprite->anchorY = anchorY;
    sprite_component_mark_transform_dirty(sprite);
}

bool sprite_component_play_animation(SpriteComponent* sprite, SpriteAnimation* animation) {
    if (!sprite || !animation) {
        return false;
    }
    
    sprite->animation = animation;
    sprite->animationTime = 0.0f;
    sprite->currentFrame = 0;
    sprite->animationPlaying = true;
    sprite->animationLooping = animation->looping;
    
    // Set first frame
    if (animation->frameCount > 0) {
        SpriteFrame* frame = &animation->frames[0];
        sprite_component_set_source_rect(sprite, 
                                        frame->sourceRect.x, frame->sourceRect.y,
                                        frame->sourceRect.width, frame->sourceRect.height);
        sprite_component_set_anchor(sprite, frame->anchorX, frame->anchorY);
    }
    
    return true;
}

void sprite_component_set_visible(SpriteComponent* sprite, bool visible) {
    if (sprite) {
        sprite->visible = visible;
    }
}

// Helper function to update animation frame
static void sprite_component_update_animation_frame(SpriteComponent* sprite) {
    if (!sprite->animation || sprite->animation->frameCount == 0) {
        return;
    }
    
    // Find current frame based on time
    float accumulatedTime = 0.0f;
    uint32_t frameIndex = 0;
    
    for (uint32_t i = 0; i < sprite->animation->frameCount; i++) {
        accumulatedTime += sprite->animation->frames[i].duration;
        if (sprite->animationTime < accumulatedTime) {
            frameIndex = i;
            break;
        }
    }
    
    // Update frame if changed
    if (frameIndex != sprite->currentFrame) {
        sprite->currentFrame = frameIndex;
        SpriteFrame* frame = &sprite->animation->frames[frameIndex];
        
        sprite_component_set_source_rect(sprite,
                                        frame->sourceRect.x, frame->sourceRect.y,
                                        frame->sourceRect.width, frame->sourceRect.height);
        sprite_component_set_anchor(sprite, frame->anchorX, frame->anchorY);
    }
}

// Helper function to update transform cache
static void sprite_component_update_transform_cache(SpriteComponent* sprite) {
    if (!sprite || !sprite->base.gameObject || !sprite->base.gameObject->transform) {
        return;
    }
    
    TransformComponent* transform = sprite->base.gameObject->transform;
    
    // Get world position
    transform_component_get_position(transform, &sprite->renderX, &sprite->renderY);
    
    // Get render size
    sprite->renderWidth = (float)sprite->sourceRect.width;
    sprite->renderHeight = (float)sprite->sourceRect.height;
    
    // Apply transform scale
    float scaleX, scaleY;
    transform_component_get_scale(transform, &scaleX, &scaleY);
    sprite->renderWidth *= scaleX;
    sprite->renderHeight *= scaleY;
    
    sprite->transformDirty = false;
}
```

### Step 3: Sprite Renderer (Batch System)

```c
// sprite_renderer.h
#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include "sprite_component.h"
#include "spatial_grid.h"
#include "pd_api.h"

#define MAX_SPRITES_PER_BATCH 256
#define MAX_RENDER_LAYERS 16

// Sprite batch for efficient rendering
typedef struct SpriteBatch {
    SpriteComponent** sprites;     // Array of sprites to render
    uint32_t spriteCount;         // Number of sprites in batch
    uint32_t capacity;            // Maximum sprites in batch
    uint32_t renderLayer;         // Layer this batch renders
    LCDBitmap* sharedBitmap;      // Shared bitmap for batching optimization
} SpriteBatch;

// Sprite renderer system
typedef struct SpriteRenderer {
    SpriteBatch batches[MAX_RENDER_LAYERS]; // One batch per layer
    uint32_t activeBatches;                 // Number of active batches
    
    // Culling support
    SpatialGrid* spatialGrid;               // For frustum culling
    LCDRect viewportRect;                   // Camera viewport
    bool frustumCullingEnabled;
    
    // Performance statistics
    uint32_t spritesRendered;
    uint32_t spritesculled;
    uint32_t drawCalls;
    float lastRenderTime;
    
    // Playdate API reference
    PlaydateAPI* pd;
    
} SpriteRenderer;

// Renderer management
SpriteRenderer* sprite_renderer_create(PlaydateAPI* pd, SpatialGrid* spatialGrid);
void sprite_renderer_destroy(SpriteRenderer* renderer);

// Rendering operations
void sprite_renderer_begin_frame(SpriteRenderer* renderer);
void sprite_renderer_render_sprites(SpriteRenderer* renderer, SpriteComponent** sprites, uint32_t count);
void sprite_renderer_end_frame(SpriteRenderer* renderer);

// Viewport and culling
void sprite_renderer_set_viewport(SpriteRenderer* renderer, int x, int y, int width, int height);
void sprite_renderer_enable_frustum_culling(SpriteRenderer* renderer, bool enabled);

// Performance and debugging
void sprite_renderer_print_stats(SpriteRenderer* renderer);
void sprite_renderer_reset_stats(SpriteRenderer* renderer);

#endif // SPRITE_RENDERER_H
```

## Unit Tests

### Sprite Component Tests

```c
// tests/components/test_sprite_component.c
#include "sprite_component.h"
#include "game_object.h"
#include <assert.h>
#include <stdio.h>

// Mock Playdate API for testing
static LCDBitmap* mock_bitmap = (LCDBitmap*)0x12345678;

void test_sprite_component_creation(void) {
    component_registry_init();
    sprite_component_register();
    
    Scene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create(scene);
    
    SpriteComponent* sprite = sprite_component_create(gameObject);
    assert(sprite != NULL);
    assert(sprite->base.type == COMPONENT_TYPE_SPRITE);
    assert(sprite->visible == true);
    assert(sprite->opacity == 1.0f);
    assert(sprite->anchorX == 0.5f && sprite->anchorY == 0.5f);
    assert(sprite->renderLayer == 0);
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Sprite component creation test passed\n");
}

void test_sprite_bitmap_management(void) {
    component_registry_init();
    sprite_component_register();
    
    Scene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create(scene);
    SpriteComponent* sprite = sprite_component_create(gameObject);
    
    // Test bitmap setting
    bool result = sprite_component_set_bitmap(sprite, mock_bitmap);
    assert(result == true);
    assert(sprite_component_get_bitmap(sprite) == mock_bitmap);
    
    // Test source rectangle
    sprite_component_set_source_rect(sprite, 10, 20, 64, 48);
    LCDRect rect = sprite_component_get_source_rect(sprite);
    assert(rect.x == 10 && rect.y == 20);
    assert(rect.width == 64 && rect.height == 48);
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Sprite bitmap management test passed\n");
}

void test_sprite_rendering_properties(void) {
    component_registry_init();
    sprite_component_register();
    
    Scene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create(scene);
    SpriteComponent* sprite = sprite_component_create(gameObject);
    
    // Test anchor
    sprite_component_set_anchor(sprite, 0.0f, 1.0f);
    float anchorX, anchorY;
    sprite_component_get_anchor(sprite, &anchorX, &anchorY);
    assert(anchorX == 0.0f && anchorY == 1.0f);
    
    // Test opacity
    sprite_component_set_opacity(sprite, 0.5f);
    assert(sprite_component_get_opacity(sprite) == 0.5f);
    
    // Test visibility
    sprite_component_set_visible(sprite, false);
    assert(sprite_component_is_visible(sprite) == false);
    
    // Test render layer
    sprite_component_set_render_layer(sprite, 5);
    assert(sprite_component_get_render_layer(sprite) == 5);
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Sprite rendering properties test passed\n");
}

void test_sprite_animation_system(void) {
    component_registry_init();
    sprite_component_register();
    
    Scene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create(scene);
    SpriteComponent* sprite = sprite_component_create(gameObject);
    
    // Create test animation
    SpriteAnimation animation;
    strcpy(animation.name, "test_anim");
    animation.frameCount = 3;
    animation.looping = true;
    animation.nextAnimation = NULL;
    
    SpriteFrame frames[3] = {
        {{0, 0, 32, 32}, 0.1f, 0.5f, 0.5f},
        {{32, 0, 32, 32}, 0.1f, 0.5f, 0.5f},
        {{64, 0, 32, 32}, 0.1f, 0.5f, 0.5f}
    };
    animation.frames = frames;
    animation.totalDuration = 0.3f;
    
    // Test animation playback
    bool result = sprite_component_play_animation(sprite, &animation);
    assert(result == true);
    assert(sprite_component_is_animation_playing(sprite) == true);
    assert(sprite->currentFrame == 0);
    
    // Simulate time progression
    Component* baseComponent = (Component*)sprite;
    sprite_component_update(baseComponent, 0.15f); // Should advance to frame 1
    assert(sprite->currentFrame == 1);
    
    sprite_component_stop_animation(sprite);
    assert(sprite_component_is_animation_playing(sprite) == false);
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Sprite animation system test passed\n");
}
```

### Performance Tests

```c
// tests/components/test_sprite_perf.c
#include "sprite_component.h"
#include "sprite_renderer.h"
#include <time.h>
#include <stdio.h>

void benchmark_sprite_updates(void) {
    component_registry_init();
    sprite_component_register();
    
    Scene* scene = mock_scene_create();
    SpriteComponent* sprites[1000];
    
    // Create many sprites
    for (int i = 0; i < 1000; i++) {
        GameObject* obj = game_object_create(scene);
        sprites[i] = sprite_component_create(obj);
        sprite_component_set_bitmap(sprites[i], mock_bitmap);
        sprite_component_set_source_rect(sprites[i], 0, 0, 32, 32);
    }
    
    clock_t start = clock();
    
    // Update all sprites for 100 frames
    for (int frame = 0; frame < 100; frame++) {
        for (int i = 0; i < 1000; i++) {
            Component* baseComponent = (Component*)sprites[i];
            sprite_component_update(baseComponent, 0.016f);
        }
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000; // milliseconds
    double per_frame = time_taken / 100;
    double per_sprite = (time_taken * 1000) / (100 * 1000); // microseconds per sprite
    
    printf("Sprite updates: %.2f ms per frame, %.2f μs per sprite\n", 
           per_frame, per_sprite);
    
    // Verify performance target
    assert(per_sprite < 10); // Less than 10μs per sprite update
    
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Sprite update performance test passed\n");
}

void benchmark_sprite_rendering(void) {
    component_registry_init();
    sprite_component_register();
    
    // Mock Playdate API setup would go here
    SpriteRenderer* renderer = sprite_renderer_create(NULL, NULL);
    
    Scene* scene = mock_scene_create();
    SpriteComponent* sprites[1000];
    
    // Create sprites
    for (int i = 0; i < 1000; i++) {
        GameObject* obj = game_object_create(scene);
        sprites[i] = sprite_component_create(obj);
        sprite_component_set_bitmap(sprites[i], mock_bitmap);
        game_object_set_position(obj, i % 400, (i / 400) * 32);
    }
    
    clock_t start = clock();
    
    // Render sprites for 60 frames
    for (int frame = 0; frame < 60; frame++) {
        sprite_renderer_begin_frame(renderer);
        sprite_renderer_render_sprites(renderer, sprites, 1000);
        sprite_renderer_end_frame(renderer);
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000; // milliseconds
    double per_frame = time_taken / 60;
    
    printf("Sprite rendering: %.2f ms per frame (1000 sprites)\n", per_frame);
    
    // Verify performance target: 1000 sprites in < 33ms (30 FPS)
    assert(per_frame < 33);
    
    sprite_renderer_destroy(renderer);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Sprite rendering performance test passed\n");
}
```

## Integration Points

### Phase 3 Integration (GameObject & Transform)
- Sprite component uses TransformComponent for world position and scale
- Transform changes trigger sprite render cache updates
- Anchor point calculations relative to transform

### Phase 4 Integration (Scene Management)
- Scene manages sprite component batches through component systems
- Batch rendering coordinated by scene render system
- Layer-based rendering order within scenes

### Phase 5 Integration (Spatial Partitioning)
- Frustum culling using spatial queries
- Visibility determination for sprites
- Performance optimization for large sprite counts

## Performance Targets

### Sprite Operations
- **Sprite updates**: < 10μs per animated sprite
- **Batch rendering**: 1000+ sprites at 30 FPS
- **Memory overhead**: < 5% for sprite management
- **Animation processing**: < 1ms for 100 animated sprites

### Rendering Efficiency
- **Draw call batching**: Minimize Playdate graphics calls
- **Frustum culling**: Skip off-screen sprites
- **Layer sorting**: Efficient depth-based rendering
- **Atlas optimization**: Shared texture usage

## Testing Criteria

### Unit Test Requirements
- ✅ Sprite component creation and destruction
- ✅ Bitmap loading and management
- ✅ Source rectangle and sprite sheet support
- ✅ Animation system functionality
- ✅ Rendering property management
- ✅ Visibility and culling behavior

### Performance Test Requirements
- ✅ Sprite update speed benchmarks
- ✅ Batch rendering performance
- ✅ Animation processing efficiency
- ✅ Memory usage measurements

### Integration Test Requirements
- ✅ Transform component integration
- ✅ Scene management coordination
- ✅ Spatial partitioning integration
- ✅ Playdate SDK graphics integration

## Success Criteria

### Functional Requirements
- [ ] Sprite component with full Playdate graphics integration
- [ ] Sprite sheet and animation support
- [ ] Batch rendering system for performance
- [ ] Frustum culling and visibility management
- [ ] Layer-based rendering with depth sorting

### Performance Requirements
- [ ] 1000+ sprites rendered at 30 FPS
- [ ] < 10μs animation updates per sprite
- [ ] Efficient batch rendering with minimal draw calls
- [ ] < 5% memory overhead for sprite management

### Quality Requirements
- [ ] 100% unit test coverage for sprite system
- [ ] Performance benchmarks meet all targets
- [ ] Clean integration with transform and scene systems
- [ ] Robust error handling and resource management

## Next Steps

Upon completion of this phase:
1. Verify all sprite component tests pass
2. Confirm performance benchmarks meet targets
3. Test integration with Playdate SDK graphics
4. Proceed to Phase 7: Collision Component implementation
5. Begin implementing collision detection with sprite bounds

This phase provides the visual foundation for the engine, enabling efficient sprite-based rendering with animation support optimized for the Playdate platform.