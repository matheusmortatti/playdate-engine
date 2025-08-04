# Phase 7: Collision Component

## Objective

Implement a comprehensive collision detection system with AABB (Axis-Aligned Bounding Box) collision detection, spatial integration, and physics response. This system targets 10,000+ collision checks per frame while maintaining efficient spatial queries and providing flexible collision response mechanisms.

## Prerequisites

- **Phase 1**: Memory Management (ObjectPool system)
- **Phase 2**: Component System (Component architecture)
- **Phase 3**: GameObject & Transform (Core entities)
- **Phase 4**: Scene Management (Scene organization)
- **Phase 5**: Spatial Partitioning (Spatial optimization)
- **Phase 6**: Sprite Component (Visual representation)
- Understanding of collision detection algorithms
- Knowledge of physics and response systems

## Technical Specifications

### Performance Targets
- **Collision detection**: 10,000+ checks per frame
- **Spatial queries**: Integration with spatial grid for broad-phase
- **Response calculation**: < 5µs per collision pair
- **Memory efficiency**: < 8% overhead for collision data
- **Update performance**: < 100ns per collision component update

### Collision System Architecture Goals
- **AABB collision detection**: Fast rectangular collision detection
- **Spatial integration**: Efficient broad-phase collision filtering
- **Collision response**: Configurable response types (trigger, physics, etc.)
- **Layer-based filtering**: Collision layer and mask system
- **Event system**: Collision enter/exit/stay callbacks

## Code Structure

```
src/components/
├── collision_component.h    # Collision component interface
├── collision_component.c    # Collision implementation
├── collision_system.h       # Collision detection system
├── collision_system.c       # System implementation
├── collision_response.h     # Physics response handling
└── collision_response.c     # Response implementation

src/physics/
├── aabb.h                   # AABB math utilities
├── aabb.c                   # AABB implementation
├── collision_layers.h       # Layer management
└── collision_layers.c       # Layer implementation

tests/components/
├── test_collision_component.c # Collision component tests
├── test_collision_system.c    # System tests
├── test_aabb.c               # AABB math tests
└── test_collision_perf.c     # Performance benchmarks

examples/
├── collision_demo.c         # Basic collision usage
├── physics_demo.c           # Physics response demo
└── layer_filtering.c        # Collision layer examples
```

## Implementation Steps

### Step 1: AABB Math Utilities

```c
// aabb.h
#ifndef AABB_H
#define AABB_H

#include <stdint.h>
#include <stdbool.h>

// Axis-Aligned Bounding Box structure
typedef struct AABB {
    float minX, minY;           // Minimum coordinates
    float maxX, maxY;           // Maximum coordinates
} AABB;

// AABB construction
AABB aabb_create(float x, float y, float width, float height);
AABB aabb_create_centered(float centerX, float centerY, float width, float height);
AABB aabb_create_from_points(float x1, float y1, float x2, float y2);

// AABB properties
float aabb_get_width(const AABB* aabb);
float aabb_get_height(const AABB* aabb);
float aabb_get_center_x(const AABB* aabb);
float aabb_get_center_y(const AABB* aabb);
float aabb_get_area(const AABB* aabb);

// AABB operations
bool aabb_intersects(const AABB* a, const AABB* b);
bool aabb_contains_point(const AABB* aabb, float x, float y);
bool aabb_contains_aabb(const AABB* container, const AABB* contained);
AABB aabb_union(const AABB* a, const AABB* b);
AABB aabb_intersection(const AABB* a, const AABB* b);

// AABB transformations
AABB aabb_translate(const AABB* aabb, float dx, float dy);
AABB aabb_scale(const AABB* aabb, float scaleX, float scaleY);
AABB aabb_expand(const AABB* aabb, float margin);

// Distance and overlap calculations
float aabb_distance_squared(const AABB* a, const AABB* b);
float aabb_overlap_x(const AABB* a, const AABB* b);
float aabb_overlap_y(const AABB* a, const AABB* b);
void aabb_get_separation_vector(const AABB* a, const AABB* b, float* dx, float* dy);

// Fast inline helpers
static inline bool aabb_is_valid(const AABB* aabb) {
    return aabb && aabb->minX <= aabb->maxX && aabb->minY <= aabb->maxY;
}

static inline bool aabb_intersects_fast(const AABB* a, const AABB* b) {
    return !(a->maxX < b->minX || a->minX > b->maxX || 
             a->maxY < b->minY || a->minY > b->maxY);
}

#endif // AABB_H
```

### Step 2: Collision Component Structure

```c
// collision_component.h
#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

#include "component.h"
#include "aabb.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct CollisionComponent CollisionComponent;
typedef struct CollisionInfo CollisionInfo;
typedef struct GameObject GameObject;

// Collision types
typedef enum {
    COLLISION_TYPE_TRIGGER = 0,    // No physics response, events only
    COLLISION_TYPE_STATIC,         // Immovable object
    COLLISION_TYPE_KINEMATIC,      // Moves but not affected by physics
    COLLISION_TYPE_DYNAMIC         // Full physics response
} CollisionType;

// Collision layers (32-bit mask)
typedef uint32_t CollisionLayer;

// Collision information for events
typedef struct CollisionInfo {
    GameObject* other;             // Other GameObject in collision
    CollisionComponent* otherCollision; // Other collision component
    float penetrationX;            // Penetration depth X
    float penetrationY;            // Penetration depth Y
    float normalX, normalY;        // Collision normal
    bool isFirstContact;           // True if this is first frame of contact
} CollisionInfo;

// Collision event callbacks
typedef void (*CollisionCallback)(CollisionComponent* self, const CollisionInfo* info);

// Collision component structure (96 bytes target)
typedef struct CollisionComponent {
    Component base;                // 32 bytes - base component
    
    // Collision bounds
    AABB localBounds;             // 16 bytes - Local space bounding box
    AABB worldBounds;             // 16 bytes - World space bounding box (cached)
    float offsetX, offsetY;       // 8 bytes - Offset from transform center
    
    // Collision properties
    CollisionType type;           // 4 bytes - Collision response type
    CollisionLayer layer;         // 4 bytes - Which layer this object is on
    CollisionLayer mask;          // 4 bytes - Which layers this object collides with
    
    // Physics properties
    float mass;                   // 4 bytes - Mass for physics response
    float restitution;            // 4 bytes - Bounciness (0.0 - 1.0)
    float friction;               // 4 bytes - Surface friction
    bool isTrigger;               // 1 byte - Trigger-only collision
    
    // State tracking
    bool boundsNeedUpdate;        // 1 byte - World bounds cache dirty
    bool wasCollidingLastFrame;   // 1 byte - For collision exit events
    uint8_t padding[1];           // 1 byte - Alignment padding
    
    // Event callbacks
    CollisionCallback onCollisionEnter; // 8 bytes - First contact callback
    CollisionCallback onCollisionStay;  // 8 bytes - Ongoing contact callback
    CollisionCallback onCollisionExit;  // 8 bytes - Contact end callback
    
} CollisionComponent;

// Collision component interface
CollisionComponent* collision_component_create(GameObject* gameObject);
void collision_component_destroy(CollisionComponent* collision);

// Bounds management
void collision_component_set_bounds(CollisionComponent* collision, float width, float height);
void collision_component_set_bounds_rect(CollisionComponent* collision, float x, float y, float width, float height);
void collision_component_set_offset(CollisionComponent* collision, float offsetX, float offsetY);
AABB collision_component_get_world_bounds(CollisionComponent* collision);
AABB collision_component_get_local_bounds(CollisionComponent* collision);

// Collision properties
void collision_component_set_type(CollisionComponent* collision, CollisionType type);
CollisionType collision_component_get_type(CollisionComponent* collision);
void collision_component_set_layer(CollisionComponent* collision, CollisionLayer layer);
CollisionLayer collision_component_get_layer(CollisionComponent* collision);
void collision_component_set_mask(CollisionComponent* collision, CollisionLayer mask);
CollisionLayer collision_component_get_mask(CollisionComponent* collision);

// Physics properties
void collision_component_set_mass(CollisionComponent* collision, float mass);
float collision_component_get_mass(CollisionComponent* collision);
void collision_component_set_restitution(CollisionComponent* collision, float restitution);
float collision_component_get_restitution(CollisionComponent* collision);
void collision_component_set_trigger(CollisionComponent* collision, bool isTrigger);
bool collision_component_is_trigger(CollisionComponent* collision);

// Event handling
void collision_component_set_collision_enter_callback(CollisionComponent* collision, CollisionCallback callback);
void collision_component_set_collision_stay_callback(CollisionComponent* collision, CollisionCallback callback);
void collision_component_set_collision_exit_callback(CollisionComponent* collision, CollisionCallback callback);

// Collision queries
bool collision_component_intersects(CollisionComponent* a, CollisionComponent* b);
bool collision_component_contains_point(CollisionComponent* collision, float x, float y);

// Component system integration
void collision_component_register(void);

// Fast access helpers
static inline bool collision_component_should_collide(CollisionComponent* a, CollisionComponent* b) {
    return (a->layer & b->mask) != 0 && (b->layer & a->mask) != 0;
}

static inline void collision_component_mark_bounds_dirty(CollisionComponent* collision) {
    if (collision) collision->boundsNeedUpdate = true;
}

#endif // COLLISION_COMPONENT_H
```

### Step 3: Collision Component Implementation

```c
// collision_component.c
#include "collision_component.h"
#include "component_registry.h"
#include "game_object.h"
#include "transform_component.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Collision component vtable
static void collision_component_init(Component* component, GameObject* gameObject);
static void collision_component_destroy_impl(Component* component);
static void collision_component_update(Component* component, float deltaTime);

static const ComponentVTable collisionVTable = {
    .init = collision_component_init,
    .destroy = collision_component_destroy_impl,
    .update = collision_component_update,
    .clone = NULL,
    .fixedUpdate = NULL,
    .render = NULL,
    .onEnabled = NULL,
    .onDisabled = NULL,
    .onGameObjectDestroyed = NULL,
    .getSerializedSize = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

void collision_component_register(void) {
    component_registry_register_type(
        COMPONENT_TYPE_COLLISION,
        sizeof(CollisionComponent),
        1000, // Pool size
        &collisionVTable,
        "Collision"
    );
}

CollisionComponent* collision_component_create(GameObject* gameObject) {
    Component* baseComponent = component_registry_create(COMPONENT_TYPE_COLLISION, gameObject);
    return (CollisionComponent*)baseComponent;
}

static void collision_component_init(Component* component, GameObject* gameObject) {
    CollisionComponent* collision = (CollisionComponent*)component;
    
    // Initialize bounds
    collision->localBounds = aabb_create(0, 0, 32, 32); // Default 32x32 bounds
    collision->worldBounds = collision->localBounds;
    collision->offsetX = 0.0f;
    collision->offsetY = 0.0f;
    
    // Initialize collision properties
    collision->type = COLLISION_TYPE_DYNAMIC;
    collision->layer = 1;          // Default layer
    collision->mask = 0xFFFFFFFF;  // Collide with all layers by default
    
    // Initialize physics properties
    collision->mass = 1.0f;
    collision->restitution = 0.0f;
    collision->friction = 0.5f;
    collision->isTrigger = false;
    
    // Initialize state
    collision->boundsNeedUpdate = true;
    collision->wasCollidingLastFrame = false;
    
    // Initialize callbacks to NULL
    collision->onCollisionEnter = NULL;
    collision->onCollisionStay = NULL;
    collision->onCollisionExit = NULL;
}

static void collision_component_destroy_impl(Component* component) {
    CollisionComponent* collision = (CollisionComponent*)component;
    
    // Clear callbacks
    collision->onCollisionEnter = NULL;
    collision->onCollisionStay = NULL;
    collision->onCollisionExit = NULL;
}

static void collision_component_update(Component* component, float deltaTime) {
    CollisionComponent* collision = (CollisionComponent*)component;
    
    // Update world bounds if transform changed
    if (collision->boundsNeedUpdate) {
        collision_component_update_world_bounds(collision);
    }
}

void collision_component_set_bounds(CollisionComponent* collision, float width, float height) {
    if (!collision) return;
    
    collision->localBounds = aabb_create_centered(0, 0, width, height);
    collision_component_mark_bounds_dirty(collision);
}

void collision_component_set_bounds_rect(CollisionComponent* collision, float x, float y, float width, float height) {
    if (!collision) return;
    
    collision->localBounds = aabb_create(x, y, width, height);
    collision_component_mark_bounds_dirty(collision);
}

void collision_component_set_offset(CollisionComponent* collision, float offsetX, float offsetY) {
    if (!collision) return;
    
    collision->offsetX = offsetX;
    collision->offsetY = offsetY;
    collision_component_mark_bounds_dirty(collision);
}

AABB collision_component_get_world_bounds(CollisionComponent* collision) {
    if (!collision) {
        return aabb_create(0, 0, 0, 0);
    }
    
    if (collision->boundsNeedUpdate) {
        collision_component_update_world_bounds(collision);
    }
    
    return collision->worldBounds;
}

bool collision_component_intersects(CollisionComponent* a, CollisionComponent* b) {
    if (!a || !b) {
        return false;
    }
    
    // Check if objects should collide based on layers
    if (!collision_component_should_collide(a, b)) {
        return false;
    }
    
    AABB boundsA = collision_component_get_world_bounds(a);
    AABB boundsB = collision_component_get_world_bounds(b);
    
    return aabb_intersects_fast(&boundsA, &boundsB);
}

bool collision_component_contains_point(CollisionComponent* collision, float x, float y) {
    if (!collision) {
        return false;
    }
    
    AABB bounds = collision_component_get_world_bounds(collision);
    return aabb_contains_point(&bounds, x, y);
}

void collision_component_set_type(CollisionComponent* collision, CollisionType type) {
    if (collision) {
        collision->type = type;
    }
}

void collision_component_set_layer(CollisionComponent* collision, CollisionLayer layer) {
    if (collision) {
        collision->layer = layer;
    }
}

void collision_component_set_mask(CollisionComponent* collision, CollisionLayer mask) {
    if (collision) {
        collision->mask = mask;
    }
}

void collision_component_set_trigger(CollisionComponent* collision, bool isTrigger) {
    if (collision) {
        collision->isTrigger = isTrigger;
    }
}

// Helper function to update world bounds from transform
static void collision_component_update_world_bounds(CollisionComponent* collision) {
    if (!collision || !collision->base.gameObject || !collision->base.gameObject->transform) {
        return;
    }
    
    TransformComponent* transform = collision->base.gameObject->transform;
    
    // Get world position
    float worldX, worldY;
    transform_component_get_position(transform, &worldX, &worldY);
    
    // Apply offset
    worldX += collision->offsetX;
    worldY += collision->offsetY;
    
    // Get scale
    float scaleX, scaleY;
    transform_component_get_scale(transform, &scaleX, &scaleY);
    
    // Transform local bounds to world space
    float width = aabb_get_width(&collision->localBounds) * scaleX;
    float height = aabb_get_height(&collision->localBounds) * scaleY;
    
    collision->worldBounds = aabb_create_centered(worldX, worldY, width, height);
    collision->boundsNeedUpdate = false;
}
```

### Step 4: Collision Detection System

```c
// collision_system.h
#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "collision_component.h"
#include "spatial_grid.h"

#define MAX_COLLISION_PAIRS 1000
#define MAX_COLLISIONS_PER_OBJECT 16

// Collision pair for tracking active collisions
typedef struct CollisionPair {
    CollisionComponent* a;
    CollisionComponent* b;
    bool wasColliding;           // Was colliding last frame
    float lastContactTime;       // Time of last contact
} CollisionPair;

// Collision detection system
typedef struct CollisionSystem {
    CollisionPair* activePairs;     // Currently active collision pairs
    uint32_t pairCount;             // Number of active pairs
    uint32_t maxPairs;              // Maximum pairs supported
    
    SpatialGrid* spatialGrid;       // For broad-phase collision detection
    
    // Performance statistics
    uint32_t broadPhaseQueries;
    uint32_t narrowPhaseChecks;
    uint32_t activeCollisions;
    float lastUpdateTime;
    
    // Configuration
    bool enableSpatialOptimization;
    float spatialQueryRadius;
    
} CollisionSystem;

// System management
CollisionSystem* collision_system_create(SpatialGrid* spatialGrid);
void collision_system_destroy(CollisionSystem* system);

// Collision detection
void collision_system_update(CollisionSystem* system, CollisionComponent** components, uint32_t count, float deltaTime);
void collision_system_check_collisions(CollisionSystem* system, CollisionComponent** components, uint32_t count);

// Collision response
void collision_system_resolve_collision(CollisionComponent* a, CollisionComponent* b, const CollisionInfo* info);

// Queries
uint32_t collision_system_query_point(CollisionSystem* system, float x, float y, CollisionComponent** results, uint32_t maxResults);
uint32_t collision_system_query_bounds(CollisionSystem* system, const AABB* bounds, CollisionComponent** results, uint32_t maxResults);

// Performance and debugging
void collision_system_print_stats(CollisionSystem* system);
void collision_system_reset_stats(CollisionSystem* system);

#endif // COLLISION_SYSTEM_H
```

## Unit Tests

### Collision Component Tests

```c
// tests/components/test_collision_component.c
#include "collision_component.h"
#include "aabb.h"
#include "game_object.h"
#include <assert.h>
#include <stdio.h>

void test_collision_component_creation(void) {
    component_registry_init();
    collision_component_register();
    transform_component_register();
    
    Scene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create(scene);
    
    CollisionComponent* collision = collision_component_create(gameObject);
    assert(collision != NULL);
    assert(collision->base.type == COMPONENT_TYPE_COLLISION);
    assert(collision->type == COLLISION_TYPE_DYNAMIC);
    assert(collision->mass == 1.0f);
    assert(collision->layer == 1);
    assert(collision->mask == 0xFFFFFFFF);
    assert(collision->isTrigger == false);
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Collision component creation test passed\n");
}

void test_collision_bounds_management(void) {
    component_registry_init();
    collision_component_register();
    transform_component_register();
    
    Scene* scene = mock_scene_create();
    GameObject* gameObject = game_object_create(scene);
    CollisionComponent* collision = collision_component_create(gameObject);
    
    // Test bounds setting
    collision_component_set_bounds(collision, 64, 48);
    AABB localBounds = collision_component_get_local_bounds(collision);
    assert(aabb_get_width(&localBounds) == 64);
    assert(aabb_get_height(&localBounds) == 48);
    
    // Test world bounds with transform
    game_object_set_position(gameObject, 100, 200);
    AABB worldBounds = collision_component_get_world_bounds(collision);
    assert(aabb_get_center_x(&worldBounds) == 100);
    assert(aabb_get_center_y(&worldBounds) == 200);
    assert(aabb_get_width(&worldBounds) == 64);
    assert(aabb_get_height(&worldBounds) == 48);
    
    // Test offset
    collision_component_set_offset(collision, 10, -5);
    worldBounds = collision_component_get_world_bounds(collision);
    assert(aabb_get_center_x(&worldBounds) == 110); // 100 + 10
    assert(aabb_get_center_y(&worldBounds) == 195); // 200 - 5
    
    game_object_destroy(gameObject);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Collision bounds management test passed\n");
}

void test_collision_detection(void) {
    component_registry_init();
    collision_component_register();
    transform_component_register();
    
    Scene* scene = mock_scene_create();
    GameObject* obj1 = game_object_create(scene);
    GameObject* obj2 = game_object_create(scene);
    
    CollisionComponent* collision1 = collision_component_create(obj1);
    CollisionComponent* collision2 = collision_component_create(obj2);
    
    // Set up collision bounds
    collision_component_set_bounds(collision1, 32, 32);
    collision_component_set_bounds(collision2, 32, 32);
    
    // Test non-intersecting objects
    game_object_set_position(obj1, 0, 0);
    game_object_set_position(obj2, 100, 100);
    assert(!collision_component_intersects(collision1, collision2));
    
    // Test intersecting objects
    game_object_set_position(obj2, 20, 20); // Overlapping
    assert(collision_component_intersects(collision1, collision2));
    
    // Test point containment
    assert(collision_component_contains_point(collision1, 5, 5));
    assert(!collision_component_contains_point(collision1, 50, 50));
    
    game_object_destroy(obj1);
    game_object_destroy(obj2);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Collision detection test passed\n");
}

void test_collision_layers(void) {
    component_registry_init();
    collision_component_register();
    transform_component_register();
    
    Scene* scene = mock_scene_create();
    GameObject* obj1 = game_object_create(scene);
    GameObject* obj2 = game_object_create(scene);
    
    CollisionComponent* collision1 = collision_component_create(obj1);
    CollisionComponent* collision2 = collision_component_create(obj2);
    
    // Set different layers
    collision_component_set_layer(collision1, 1);  // Layer 1
    collision_component_set_layer(collision2, 2);  // Layer 2
    
    // Set masks to not collide
    collision_component_set_mask(collision1, 4);   // Only collides with layer 4
    collision_component_set_mask(collision2, 8);   // Only collides with layer 8
    
    // Test layer filtering
    assert(!collision_component_should_collide(collision1, collision2));
    
    // Set masks to collide
    collision_component_set_mask(collision1, 2);   // Collides with layer 2
    collision_component_set_mask(collision2, 1);   // Collides with layer 1
    
    assert(collision_component_should_collide(collision1, collision2));
    
    game_object_destroy(obj1);
    game_object_destroy(obj2);
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Collision layer filtering test passed\n");
}
```

### AABB Math Tests

```c
// tests/components/test_aabb.c
#include "aabb.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

void test_aabb_creation(void) {
    AABB aabb1 = aabb_create(10, 20, 30, 40);
    assert(aabb1.minX == 10 && aabb1.minY == 20);
    assert(aabb1.maxX == 40 && aabb1.maxY == 60); // 10+30, 20+40
    
    AABB aabb2 = aabb_create_centered(50, 60, 20, 30);
    assert(aabb2.minX == 40 && aabb2.minY == 45); // 50-10, 60-15
    assert(aabb2.maxX == 60 && aabb2.maxY == 75); // 50+10, 60+15
    
    assert(aabb_get_width(&aabb1) == 30);
    assert(aabb_get_height(&aabb1) == 40);
    assert(aabb_get_center_x(&aabb2) == 50);
    assert(aabb_get_center_y(&aabb2) == 60);
    
    printf("✓ AABB creation test passed\n");
}

void test_aabb_intersection(void) {
    AABB aabb1 = aabb_create(0, 0, 50, 50);
    AABB aabb2 = aabb_create(25, 25, 50, 50);
    AABB aabb3 = aabb_create(100, 100, 50, 50);
    
    // Test intersection
    assert(aabb_intersects(&aabb1, &aabb2));
    assert(!aabb_intersects(&aabb1, &aabb3));
    
    // Test fast intersection
    assert(aabb_intersects_fast(&aabb1, &aabb2));
    assert(!aabb_intersects_fast(&aabb1, &aabb3));
    
    // Test point containment
    assert(aabb_contains_point(&aabb1, 25, 25));
    assert(!aabb_contains_point(&aabb1, 75, 75));
    
    printf("✓ AABB intersection test passed\n");
}

void test_aabb_operations(void) {
    AABB aabb1 = aabb_create(0, 0, 50, 50);
    AABB aabb2 = aabb_create(25, 25, 50, 50);
    
    // Test union
    AABB unionAABB = aabb_union(&aabb1, &aabb2);
    assert(unionAABB.minX == 0 && unionAABB.minY == 0);
    assert(unionAABB.maxX == 75 && unionAABB.maxY == 75);
    
    // Test intersection
    AABB intersectionAABB = aabb_intersection(&aabb1, &aabb2);
    assert(intersectionAABB.minX == 25 && intersectionAABB.minY == 25);
    assert(intersectionAABB.maxX == 50 && intersectionAABB.maxY == 50);
    
    // Test translation
    AABB translatedAABB = aabb_translate(&aabb1, 10, 20);
    assert(translatedAABB.minX == 10 && translatedAABB.minY == 20);
    assert(translatedAABB.maxX == 60 && translatedAABB.maxY == 70);
    
    printf("✓ AABB operations test passed\n");
}

void test_aabb_distance_and_overlap(void) {
    AABB aabb1 = aabb_create(0, 0, 50, 50);
    AABB aabb2 = aabb_create(25, 25, 50, 50);
    
    // Test overlap calculations
    float overlapX = aabb_overlap_x(&aabb1, &aabb2);
    float overlapY = aabb_overlap_y(&aabb1, &aabb2);
    assert(overlapX == 25); // 50 - 25
    assert(overlapY == 25); // 50 - 25
    
    // Test separation vector
    float dx, dy;
    aabb_get_separation_vector(&aabb1, &aabb2, &dx, &dy);
    // Should give minimum separation to resolve collision
    assert(fabsf(dx) == 25 || fabsf(dy) == 25);
    
    printf("✓ AABB distance and overlap test passed\n");
}
```

### Performance Tests

```c
// tests/components/test_collision_perf.c
#include "collision_component.h"
#include "collision_system.h"
#include <time.h>
#include <stdio.h>

void benchmark_collision_detection(void) {
    component_registry_init();
    collision_component_register();
    transform_component_register();
    
    Scene* scene = mock_scene_create();
    CollisionComponent* collisions[1000];
    
    // Create many collision components
    for (int i = 0; i < 1000; i++) {
        GameObject* obj = game_object_create(scene);
        collisions[i] = collision_component_create(obj);
        collision_component_set_bounds(collisions[i], 32, 32);
        game_object_set_position(obj, i % 200 * 16, (i / 200) * 16);
    }
    
    clock_t start = clock();
    
    // Perform collision checks
    uint32_t collisionCount = 0;
    for (int i = 0; i < 1000; i++) {
        for (int j = i + 1; j < 1000; j++) {
            if (collision_component_intersects(collisions[i], collisions[j])) {
                collisionCount++;
            }
        }
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000; // microseconds
    double per_check = time_taken / (1000 * 999 / 2); // Total checks
    
    printf("Collision detection: %.2f μs total, %.2f ns per check (%d collisions found)\n", 
           time_taken, per_check * 1000, collisionCount);
    
    // Verify performance target
    assert(per_check < 5); // Less than 5μs per collision check
    
    mock_scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Collision detection performance test passed\n");
}

void benchmark_aabb_operations(void) {
    AABB aabb1 = aabb_create(0, 0, 50, 50);
    AABB aabb2 = aabb_create(25, 25, 50, 50);
    
    clock_t start = clock();
    
    // Perform many AABB intersections
    volatile bool result;
    for (int i = 0; i < 1000000; i++) {
        result = aabb_intersects_fast(&aabb1, &aabb2);
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000; // microseconds
    double per_check = time_taken / 1000000;
    
    printf("AABB intersection: %.2f μs for 1,000,000 checks (%.2f ns per check)\n", 
           time_taken, per_check * 1000);
    
    // Should be extremely fast (sub-nanosecond)
    assert(per_check < 0.01); // Less than 10 picoseconds per check
    
    printf("✓ AABB operation performance test passed\n");
}
```

## Integration Points

### Phase 3 Integration (GameObject & Transform)
- Collision bounds automatically updated when transform changes
- World-space collision detection using transform position and scale
- Collision offset relative to GameObject center

### Phase 5 Integration (Spatial Partitioning)
- Broad-phase collision detection using spatial grid queries
- Efficient neighbor finding for collision candidates
- Performance optimization for large numbers of collision objects

### Phase 6 Integration (Sprite Component)
- Collision bounds can be automatically derived from sprite dimensions
- Visual debugging of collision bounds over sprites
- Collision-based sprite visibility optimizations

## Performance Targets

### Collision Operations
- **AABB intersection**: < 1ns per check
- **Collision component intersection**: < 5μs per pair
- **Broad-phase filtering**: 10,000+ objects with spatial optimization
- **Response calculation**: < 5μs per collision resolution

### System Efficiency
- **Update performance**: < 100ns per collision component update
- **Memory overhead**: < 8% for collision data structures
- **Spatial integration**: O(log n) collision candidate filtering

## Testing Criteria

### Unit Test Requirements
- ✅ Collision component creation and property management
- ✅ AABB math operations and accuracy
- ✅ Collision detection between components
- ✅ Layer-based collision filtering
- ✅ Transform integration and world bounds
- ✅ Trigger vs. physics collision behavior

### Performance Test Requirements
- ✅ Large-scale collision detection benchmarks
- ✅ AABB operation speed validation
- ✅ Spatial optimization performance
- ✅ Memory usage measurements

### Integration Test Requirements
- ✅ Transform component integration
- ✅ Spatial partitioning system coordination
- ✅ Component system batch processing
- ✅ Event callback system functionality

## Success Criteria

### Functional Requirements
- [ ] AABB collision detection with sub-pixel accuracy
- [ ] Layer-based collision filtering system
- [ ] Trigger and physics collision type support
- [ ] Collision event callbacks (enter/stay/exit)
- [ ] Spatial grid integration for performance

### Performance Requirements
- [ ] 10,000+ collision checks per frame capability
- [ ] < 5μs collision detection per pair
- [ ] < 100ns collision component updates
- [ ] < 8% memory overhead for collision data

### Quality Requirements
- [ ] 100% unit test coverage for collision system
- [ ] Performance benchmarks meet all targets
- [ ] Robust integration with transform and spatial systems
- [ ] Comprehensive error handling and validation

## Next Steps

Upon completion of this phase:
1. Verify all collision detection tests pass
2. Confirm performance benchmarks meet targets
3. Test integration with spatial and transform systems
4. Proceed to Phase 8: Lua Bindings implementation
5. Begin implementing script-accessible collision APIs

This phase provides the foundation for all physics interactions, collision-based gameplay, and spatial relationships within the game world.