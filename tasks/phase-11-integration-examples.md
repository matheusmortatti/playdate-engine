# Phase 11: Integration Examples & Complete Games

## Objective

Develop comprehensive example projects and complete games that demonstrate the full capabilities of the Playdate engine while serving as learning resources, integration tests, and showcases for potential developers. These examples validate the engine's real-world usability and performance under typical game development scenarios.

## Prerequisites

- **Phase 1-10**: Complete engine implementation with optimization
- Game design and development knowledge
- Understanding of Playdate platform constraints and capabilities
- Asset creation skills (sprites, sounds, levels)

## Technical Specifications

### Example Project Goals
- **Real-world validation**: Test engine under actual game development conditions
- **Performance demonstration**: Show engine meeting targets in complete games
- **Learning resources**: Provide clear, documented examples for developers
- **Platform showcase**: Highlight unique Playdate features and constraints
- **Integration testing**: Validate all engine systems working together

### Project Complexity Levels
- **Basic Examples**: Simple demonstrations of individual features
- **Intermediate Projects**: Games combining multiple engine systems
- **Advanced Games**: Complex projects showcasing full engine capabilities
- **Performance Demos**: Stress tests pushing engine to its limits

## Code Structure

```
examples/
├── basic/                      # Simple feature demonstrations
│   ├── hello_world/           # Minimal engine usage
│   ├── sprite_demo/           # Basic sprite rendering
│   ├── input_handling/        # Input system usage
│   ├── collision_basics/      # Simple collision detection
│   └── lua_scripting/         # Basic Lua integration
├── intermediate/              # Multi-system games
│   ├── platformer/            # Side-scrolling platformer
│   ├── top_down_adventure/    # Zelda-style adventure
│   ├── puzzle_game/           # Grid-based puzzle game
│   ├── shooter/               # Simple shoot-em-up
│   └── rpg_battle/            # Turn-based battle system
├── advanced/                  # Complex showcase games
│   ├── metroidvania/          # Large interconnected world
│   ├── racing_game/           # Physics-heavy racing
│   ├── strategy_game/         # RTS with many units
│   └── sandbox_builder/       # Creative building game
├── performance/               # Engine stress tests
│   ├── object_stress_test/    # 10,000+ GameObjects
│   ├── collision_benchmark/   # Massive collision detection
│   ├── rendering_stress/      # 1000+ sprites rendering
│   └── memory_efficiency/     # Memory usage optimization
└── templates/                 # Project templates
    ├── empty_project/         # Minimal starting template
    ├── platformer_template/   # Platformer game template
    └── lua_game_template/     # Lua-based game template

assets/
├── sprites/                   # Shared sprite assets
├── sounds/                    # Audio assets
├── fonts/                     # Text rendering fonts
├── tilesets/                  # Tilemap assets
└── animations/                # Animation sequences

docs/examples/
├── GETTING_STARTED.md         # How to run examples
├── GAME_DEVELOPMENT_GUIDE.md  # Complete development guide
├── PERFORMANCE_TIPS.md        # Optimization best practices
└── API_REFERENCE.md           # Complete API documentation
```

## Implementation Steps

### Step 1: Basic Examples

#### Hello World Example

```c
// examples/basic/hello_world/main.c
#include "playdate_engine.h"

static Scene* g_scene;
static GameObject* g_text_object;

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
    switch (event) {
        case kEventInit:
            // Initialize engine
            engine_init(pd);
            
            // Create scene
            g_scene = scene_create("HelloWorld", 10);
            
            // Create text GameObject (would use text component in real implementation)
            g_text_object = game_object_create(g_scene);
            game_object_set_position(g_text_object, 200, 120);
            
            pd->system->setUpdateCallback(update, pd);
            break;
            
        case kEventTerminate:
            scene_destroy(g_scene);
            break;
            
        default:
            break;
    }
    
    return 0;
}

static int update(void* userdata) {
    PlaydateAPI* pd = (PlaydateAPI*)userdata;
    
    // Clear screen
    pd->graphics->clear(kColorWhite);
    
    // Update scene
    scene_update(g_scene, 1.0f / 30.0f);
    
    // Render scene
    scene_render(g_scene);
    
    // Draw hello world text
    pd->graphics->drawText("Hello, Playdate Engine!", strlen("Hello, Playdate Engine!"), 
                          kASCIIEncoding, 100, 110);
    
    return 1;
}
```

#### Sprite Demo Example

```c
// examples/basic/sprite_demo/main.c
#include "playdate_engine.h"

static Scene* g_scene;
static GameObject* g_player;
static SpriteComponent* g_player_sprite;

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
    switch (event) {
        case kEventInit:
            engine_init(pd);
            sprite_component_set_playdate_api(pd);
            
            // Create scene
            g_scene = scene_create("SpriteDemo", 100);
            
            // Create player GameObject
            g_player = game_object_create(g_scene);
            game_object_set_position(g_player, 200, 120);
            
            // Add sprite component
            g_player_sprite = sprite_component_create(g_player);
            game_object_add_component(g_player, (Component*)g_player_sprite);
            
            // Load sprite
            sprite_component_load_bitmap(g_player_sprite, "assets/player.png");
            sprite_component_set_anchor(g_player_sprite, 0.5f, 0.5f);
            
            pd->system->setUpdateCallback(update, pd);
            break;
            
        case kEventTerminate:
            scene_destroy(g_scene);
            break;
    }
    
    return 0;
}

static int update(void* userdata) {
    PlaydateAPI* pd = (PlaydateAPI*)userdata;
    
    // Handle input
    PDButtons current, pushed, released;
    pd->system->getButtonState(&current, &pushed, &released);
    
    float moveSpeed = 100.0f; // pixels per second
    float deltaTime = 1.0f / 30.0f;
    
    if (current & kButtonLeft) {
        game_object_translate(g_player, -moveSpeed * deltaTime, 0);
        sprite_component_set_flip(g_player_sprite, kBitmapFlippedX);
    }
    if (current & kButtonRight) {
        game_object_translate(g_player, moveSpeed * deltaTime, 0);
        sprite_component_set_flip(g_player_sprite, kBitmapUnflipped);
    }
    if (current & kButtonUp) {
        game_object_translate(g_player, 0, -moveSpeed * deltaTime);
    }
    if (current & kButtonDown) {
        game_object_translate(g_player, 0, moveSpeed * deltaTime);
    }
    
    // Update and render scene
    pd->graphics->clear(kColorWhite);
    scene_update(g_scene, deltaTime);
    scene_render(g_scene);
    
    return 1;
}
```

### Step 2: Intermediate Projects

#### Platformer Game

##### Game Header

```c
// examples/intermediate/platformer/game.h
#ifndef PLATFORMER_GAME_H
#define PLATFORMER_GAME_H

#include "playdate_engine.h"

// Game states
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} GameState;

// Player controller
typedef struct {
    GameObject* gameObject;
    SpriteComponent* sprite;
    CollisionComponent* collision;
    
    // Movement properties
    float velocityX, velocityY;
    float maxSpeed;
    float jumpForce;
    bool isGrounded;
    bool isJumping;
    
    // Animation state
    SpriteAnimation* idleAnimation;
    SpriteAnimation* walkAnimation;
    SpriteAnimation* jumpAnimation;
    
} PlayerController;

// Level data
typedef struct {
    TiledMap* tiledMap;
    GameObject** platforms;
    uint32_t platformCount;
    GameObject** enemies;
    uint32_t enemyCount;
    GameObject** collectibles;
    uint32_t collectibleCount;
} Level;

// Game instance
typedef struct {
    PlaydateAPI* pd;
    Scene* scene;
    GameState state;
    
    PlayerController player;
    Level currentLevel;
    
    // Game state
    uint32_t score;
    uint32_t lives;
    float gameTime;
    
    // Performance monitoring
    PerformanceMonitor* perfMonitor;
    
} PlatformerGame;
```

##### Game Functions

```c
// Game functions
PlatformerGame* platformer_game_create(PlaydateAPI* pd);
void platformer_game_destroy(PlatformerGame* game);
void platformer_game_update(PlatformerGame* game, float deltaTime);
void platformer_game_render(PlatformerGame* game);
void platformer_game_load_level(PlatformerGame* game, const char* levelPath);

// Player functions
void player_controller_init(PlayerController* player, Scene* scene);
void player_controller_update(PlayerController* player, float deltaTime, PDButtons buttons);
void player_controller_handle_collision(PlayerController* player, const CollisionInfo* info);

#endif // PLATFORMER_GAME_H
```

##### Player Controller Implementation

```c
// examples/intermediate/platformer/player.c
#include "game.h"
#include <math.h>

#define GRAVITY 980.0f          // pixels/second²
#define MAX_FALL_SPEED 600.0f   // pixels/second
#define JUMP_FORCE -400.0f      // pixels/second (negative = up)
#define WALK_SPEED 150.0f       // pixels/second
#define ACCELERATION 800.0f     // pixels/second²
#define FRICTION 600.0f         // pixels/second²

void player_controller_init(PlayerController* player, Scene* scene) {
    // Create player GameObject
    player->gameObject = game_object_create(scene);
    game_object_set_position(player->gameObject, 100, 200);
    
    // Add sprite component
    player->sprite = sprite_component_create(player->gameObject);
    game_object_add_component(player->gameObject, (Component*)player->sprite);
    
    // Load player sprite and animations
    sprite_component_load_bitmap(player->sprite, "assets/player_spritesheet.png");
    sprite_component_set_anchor(player->sprite, 0.5f, 1.0f); // Bottom-center anchor
    
    // Add collision component
    player->collision = collision_component_create(player->gameObject);
    game_object_add_component(player->gameObject, (Component*)player->collision);
    collision_component_set_bounds(player->collision, 16, 24);
    collision_component_set_type(player->collision, COLLISION_TYPE_DYNAMIC);
    collision_component_set_collision_enter_callback(player->collision, 
        (CollisionCallback)player_controller_handle_collision);
    
    // Initialize movement properties
    player->velocityX = 0.0f;
    player->velocityY = 0.0f;
    player->maxSpeed = WALK_SPEED;
    player->jumpForce = JUMP_FORCE;
    player->isGrounded = false;
    player->isJumping = false;
    
    // Load animations (simplified - would load from aseprite files)
    // player->idleAnimation = load_animation("player_idle");
    // player->walkAnimation = load_animation("player_walk");
    // player->jumpAnimation = load_animation("player_jump");
}
```

##### Player Update Function

```c
void player_controller_update(PlayerController* player, float deltaTime, PDButtons buttons) {
    // Handle horizontal movement
    float targetVelocityX = 0.0f;
    
    if (buttons & kButtonLeft) {
        targetVelocityX = -player->maxSpeed;
        sprite_component_set_flip(player->sprite, kBitmapFlippedX);
    } else if (buttons & kButtonRight) {
        targetVelocityX = player->maxSpeed;
        sprite_component_set_flip(player->sprite, kBitmapUnflipped);
    }
    
    // Apply acceleration/friction
    if (targetVelocityX != 0) {
        // Accelerate towards target velocity
        if (player->velocityX < targetVelocityX) {
            player->velocityX = fminf(player->velocityX + ACCELERATION * deltaTime, targetVelocityX);
        } else if (player->velocityX > targetVelocityX) {
            player->velocityX = fmaxf(player->velocityX - ACCELERATION * deltaTime, targetVelocityX);
        }
    } else {
        // Apply friction when no input
        if (player->velocityX > 0) {
            player->velocityX = fmaxf(0, player->velocityX - FRICTION * deltaTime);
        } else if (player->velocityX < 0) {
            player->velocityX = fminf(0, player->velocityX + FRICTION * deltaTime);
        }
    }
    
    // Handle jumping
    if ((buttons & kButtonA) && player->isGrounded && !player->isJumping) {
        player->velocityY = player->jumpForce;
        player->isJumping = true;
        player->isGrounded = false;
    }
    
    // Apply gravity
    if (!player->isGrounded) {
        player->velocityY += GRAVITY * deltaTime;
        player->velocityY = fminf(player->velocityY, MAX_FALL_SPEED);
    }
    
    // Update position
    game_object_translate(player->gameObject, 
                         player->velocityX * deltaTime, 
                         player->velocityY * deltaTime);
```

##### Animation and Collision Handling

```c
    // Update animation based on state
    if (!player->isGrounded) {
        // sprite_component_play_animation(player->sprite, player->jumpAnimation);
    } else if (fabsf(player->velocityX) > 0.1f) {
        // sprite_component_play_animation(player->sprite, player->walkAnimation);
    } else {
        // sprite_component_play_animation(player->sprite, player->idleAnimation);
    }
    
    // Reset grounded state (will be set by collision detection)
    player->isGrounded = false;
}

void player_controller_handle_collision(PlayerController* player, const CollisionInfo* info) {
    // Handle collision with platforms
    if (info->other && game_object_has_component(info->other, COMPONENT_TYPE_COLLISION)) {
        CollisionComponent* otherCollision = (CollisionComponent*)game_object_get_component(info->other, COMPONENT_TYPE_COLLISION);
        
        // Check if it's a platform
        if (collision_component_get_layer(otherCollision) == LAYER_PLATFORMS) {
            // Landing on top of platform
            if (info->normalY < -0.5f && player->velocityY > 0) {
                player->velocityY = 0;
                player->isGrounded = true;
                player->isJumping = false;
                
                // Adjust position to sit on platform
                game_object_translate(player->gameObject, 0, info->penetrationY);
            }
            // Hitting platform from below
            else if (info->normalY > 0.5f && player->velocityY < 0) {
                player->velocityY = 0;
                game_object_translate(player->gameObject, 0, info->penetrationY);
            }
            // Hitting platform from side
            else if (fabsf(info->normalX) > 0.5f) {
                player->velocityX = 0;
                game_object_translate(player->gameObject, info->penetrationX, 0);
            }
        }
    }
}
```

### Step 3: Advanced Showcase Games

#### Strategy Game with Many Units

##### Strategy Game Header

```c
// examples/advanced/strategy_game/game.h
#ifndef STRATEGY_GAME_H
#define STRATEGY_GAME_H

#include "playdate_engine.h"

#define MAX_UNITS 1000
#define MAX_BUILDINGS 200
#define MAP_WIDTH 64
#define MAP_HEIGHT 64

// Unit types
typedef enum {
    UNIT_TYPE_WORKER,
    UNIT_TYPE_SOLDIER,
    UNIT_TYPE_ARCHER,
    UNIT_TYPE_CAVALRY
} UnitType;

// Unit AI states
typedef enum {
    AI_STATE_IDLE,
    AI_STATE_MOVING,
    AI_STATE_ATTACKING,
    AI_STATE_GATHERING,
    AI_STATE_BUILDING
} AIState;

// Game unit
typedef struct {
    GameObject* gameObject;
    SpriteComponent* sprite;
    CollisionComponent* collision;
    
    UnitType type;
    uint32_t playerId;
    
    // Stats
    float health;
    float maxHealth;
    float damage;
    float speed;
    float range;
    
    // AI
    AIState aiState;
    GameObject* target;
    float targetX, targetY;
    
    // Path finding
    uint32_t* path;
    uint32_t pathLength;
    uint32_t pathIndex;
    
} Unit;
```

##### Strategy Game Instance

```c
// Game instance
typedef struct {
    PlaydateAPI* pd;
    Scene* scene;
    
    // Units and buildings
    Unit units[MAX_UNITS];
    uint32_t unitCount;
    
    // Map and pathfinding
    uint8_t map[MAP_WIDTH][MAP_HEIGHT]; // 0 = passable, 1 = blocked
    
    // Spatial optimization
    SpatialGrid* spatialGrid;
    
    // Performance monitoring
    uint32_t unitsUpdatedThisFrame;
    float aiUpdateTime;
    float pathfindingTime;
    
} StrategyGame;

// Game functions
StrategyGame* strategy_game_create(PlaydateAPI* pd);
void strategy_game_destroy(StrategyGame* game);
void strategy_game_update(StrategyGame* game, float deltaTime);
void strategy_game_render(StrategyGame* game);

// Unit management
Unit* strategy_game_create_unit(StrategyGame* game, UnitType type, float x, float y, uint32_t playerId);
void strategy_game_update_units(StrategyGame* game, float deltaTime);
void strategy_game_update_unit_ai(StrategyGame* game, Unit* unit, float deltaTime);

// Pathfinding
bool strategy_game_find_path(StrategyGame* game, float startX, float startY, float endX, float endY,
                           uint32_t** path, uint32_t* pathLength);

#endif // STRATEGY_GAME_H
```

### Step 4: Performance Stress Tests

#### Object Stress Test

```c
// examples/performance/object_stress_test/main.c
#include "playdate_engine.h"
#include "profiler.h"
#include "performance_monitor.h"

#define TARGET_OBJECT_COUNT 10000
#define SPAWN_RATE 100 // objects per second

static Scene* g_scene;
static GameObject** g_objects;
static uint32_t g_object_count;
static float g_spawn_timer;
static PerformanceMonitor* g_perf_monitor;

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
    switch (event) {
        case kEventInit:
            // Initialize engine with profiling
            engine_init(pd);
            profiler_init();
            profiler_enable(true);
            
            // Initialize performance monitoring
            g_perf_monitor = performance_monitor_create();
            performance_monitor_enable(g_perf_monitor, true);
            
            // Create scene
            g_scene = scene_create("StressTest", TARGET_OBJECT_COUNT + 1000);
            
            // Allocate object array
            g_objects = malloc(TARGET_OBJECT_COUNT * sizeof(GameObject*));
            g_object_count = 0;
            g_spawn_timer = 0;
            
            pd->system->setUpdateCallback(update, pd);
            break;
            
        case kEventTerminate:
            // Print performance report
            profiler_print_report();
            performance_monitor_print_report(g_perf_monitor);
            
            // Cleanup
            free(g_objects);
            performance_monitor_destroy(g_perf_monitor);
            scene_destroy(g_scene);
            profiler_shutdown();
            break;
    }
    
    return 0;
}
```

#### Stress Test Update Function

```c
static int update(void* userdata) {
    PROFILE_FUNCTION();
    
    PlaydateAPI* pd = (PlaydateAPI*)userdata;
    float deltaTime = 1.0f / 30.0f;
    
    // Spawn new objects
    g_spawn_timer += deltaTime;
    if (g_spawn_timer >= 1.0f / SPAWN_RATE && g_object_count < TARGET_OBJECT_COUNT) {
        PROFILE_SCOPE("Object Creation");
        
        GameObject* obj = game_object_create(g_scene);
        if (obj) {
            // Random position
            float x = (rand() % 400) + 10;
            float y = (rand() % 240) + 10;
            game_object_set_position(obj, x, y);
            
            // Add sprite component
            SpriteComponent* sprite = sprite_component_create(obj);
            game_object_add_component(obj, (Component*)sprite);
            
            g_objects[g_object_count] = obj;
            g_object_count++;
        }
        
        g_spawn_timer = 0;
    }
    
    // Update all objects
    {
        PROFILE_SCOPE("Object Updates");
        
        // Make objects move in simple patterns
        for (uint32_t i = 0; i < g_object_count; i++) {
            GameObject* obj = g_objects[i];
            if (obj) {
                float x, y;
                game_object_get_position(obj, &x, &y);
                
                // Simple circular motion
                float speed = 50.0f;
                float angle = (float)i * 0.1f + g_spawn_timer;
                x += cosf(angle) * speed * deltaTime;
                y += sinf(angle) * speed * deltaTime;
                
                // Wrap around screen
                if (x < 0) x = 400;
                if (x > 400) x = 0;
                if (y < 0) y = 240;
                if (y > 240) y = 0;
                
                game_object_set_position(obj, x, y);
            }
        }
    }
```

#### Scene Update and Rendering

```c
    // Update scene
    {
        PROFILE_SCOPE("Scene Update");
        scene_update(g_scene, deltaTime);
    }
    
    // Render
    {
        PROFILE_SCOPE("Rendering");
        pd->graphics->clear(kColorWhite);
        scene_render(g_scene);
        
        // Draw performance info
        char info[128];
        snprintf(info, sizeof(info), "Objects: %u/%u FPS: %.1f", 
                g_object_count, TARGET_OBJECT_COUNT, profiler_get_fps());
        pd->graphics->drawText(info, strlen(info), kASCIIEncoding, 10, 10);
    }
    
    // Update performance monitor
    performance_monitor_update(g_perf_monitor, deltaTime);
    
    return 1;
}
```

### Step 5: Lua Scripted Games

#### Lua Game Template

```lua
-- examples/templates/lua_game_template/main.lua
import "playdate_engine_lua"

-- Game state
local gameState = {
    scene = nil,
    player = nil,
    enemies = {},
    score = 0,
    gameTime = 0
}

-- Initialize game
function gameState.init()
    print("Initializing Lua game...")
    
    -- Create scene
    gameState.scene = Scene.new("LuaGame", 1000)
    
    -- Create player
    gameState.player = GameObject.new()
    gameState.player:setPosition(200, 120)
    
    -- Add sprite component
    local sprite = gameState.player:addComponent("Sprite")
    sprite:loadImage("assets/player.png")
    
    -- Add collision component
    local collision = gameState.player:addComponent("Collision")
    collision:setBounds(16, 16)
    
    -- Spawn some enemies
    gameState.spawnEnemies(5)
end

-- Spawn enemies
function gameState.spawnEnemies(count)
    for i = 1, count do
        local enemy = GameObject.new()
        enemy:setPosition(math.random(50, 350), math.random(50, 190))
        
        local sprite = enemy:addComponent("Sprite")
        sprite:loadImage("assets/enemy.png")
        
        local collision = enemy:addComponent("Collision")
        collision:setBounds(12, 12)
        collision:setLayer(2) -- Enemy layer
        
        table.insert(gameState.enemies, enemy)
    end
end
```

#### Game Update and Rendering

```lua
-- Update game
function gameState.update(dt)
    gameState.gameTime = gameState.gameTime + dt
    
    -- Handle player input
    local moveSpeed = 120 -- pixels per second
    
    if playdate.buttonIsPressed(playdate.kButtonLeft) then
        gameState.player:translate(-moveSpeed * dt, 0)
    end
    if playdate.buttonIsPressed(playdate.kButtonRight) then
        gameState.player:translate(moveSpeed * dt, 0)
    end
    if playdate.buttonIsPressed(playdate.kButtonUp) then
        gameState.player:translate(0, -moveSpeed * dt)
    end
    if playdate.buttonIsPressed(playdate.kButtonDown) then
        gameState.player:translate(0, moveSpeed * dt)
    end
    
    -- Update enemies (simple AI)
    for i, enemy in ipairs(gameState.enemies) do
        if enemy then
            -- Simple chase AI
            local playerX, playerY = gameState.player:getPosition()
            local enemyX, enemyY = enemy:getPosition()
            
            local dx = playerX - enemyX
            local dy = playerY - enemyY
            local distance = math.sqrt(dx * dx + dy * dy)
            
            if distance > 1 then
                local speed = 60 * dt
                enemy:translate((dx / distance) * speed, (dy / distance) * speed)
            end
        end
    end
    
    -- Update scene
    gameState.scene:update(dt)
end

-- Render game
function gameState.render()
    playdate.graphics.clear(playdate.graphics.kColorWhite)
    
    -- Render scene
    gameState.scene:render()
    
    -- Draw UI
    playdate.graphics.drawText("Score: " .. gameState.score, 10, 10)
    playdate.graphics.drawText("Time: " .. string.format("%.1f", gameState.gameTime), 10, 30)
    playdate.graphics.drawText("Enemies: " .. #gameState.enemies, 10, 50)
end

-- Playdate callbacks
function playdate.update()
    gameState.update(1/30)
end

function playdate.draw()
    gameState.render()
end

-- Initialize game
gameState.init()
```

## Documentation and Guides

### Complete Development Guide

#### Getting Started Section

```markdown
# Game Development Guide

## Getting Started

### Setting Up Your Development Environment

1. Install the Playdate SDK
2. Clone the Playdate Engine repository
3. Build the engine using the provided scripts
4. Set up your IDE with engine headers

### Your First Game

1. Start with the empty_project template
2. Initialize the engine in your main function
3. Create a scene and GameObjects
4. Add components for functionality
5. Handle input and update logic
6. Render your game world

### Engine Architecture Overview

- **GameObjects**: Containers for game entities
- **Components**: Modular functionality (Transform, Sprite, Collision)
- **Scenes**: Organize and manage GameObjects
- **Systems**: Update and render components efficiently

### Performance Best Practices

1. **Object Pooling**: Reuse GameObjects instead of creating/destroying
2. **Batch Operations**: Group similar operations together
3. **Spatial Partitioning**: Use collision layers and spatial queries
4. **Memory Management**: Monitor allocations and avoid leaks
5. **Profiling**: Use built-in profiler to identify bottlenecks
```

#### Common Patterns Section

```markdown
### Common Patterns

#### Player Controller
```c
// Handle input and movement
void update_player(PlayerController* player, float deltaTime, PDButtons buttons)
{
    // Input handling
    if (buttons & kButtonLeft) {
        player->velocityX = -player->speed;
    }
    // ... more input handling
    
    // Physics update
    player->velocityY += gravity * deltaTime;
    
    // Update position
    game_object_translate(player->gameObject, 
                         player->velocityX * deltaTime,
                         player->velocityY * deltaTime);
}
```

#### Collision Handling
```c
// Collision callback
void on_collision(CollisionComponent* self, const CollisionInfo* info)
{
    GameObject* other = info->other;
    
    if (game_object_has_component(other, COMPONENT_TYPE_ENEMY)) {
        // Handle collision with enemy
        player_take_damage(self->gameObject);
    }
}
```

#### Game State Management
```c
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} GameState;

void update_game(Game* game, float deltaTime)
{
    switch (game->state) {
        case GAME_STATE_PLAYING:
            update_gameplay(game, deltaTime);
            break;
        case GAME_STATE_PAUSED:
            update_pause_menu(game, deltaTime);
            break;
        // ... other states
    }
}
```
```

#### Advanced Topics Section

```markdown
## Advanced Topics

### Custom Components

Create your own components by extending the base Component structure:

```c
typedef struct {
    Component base;
    // Your custom data
    float customProperty;
} CustomComponent;
```

### Lua Scripting

Use Lua for rapid prototyping and gameplay logic:

```lua
-- Define behavior in Lua
function updateEnemy(enemy, deltaTime)
    -- AI logic here
end
```

### Asset Pipeline

- Use Aseprite for sprites and animations
- Use Tiled for level design
- Organize assets in logical folder structures
- Consider memory constraints (16MB total)

### Debugging and Profiling

- Use the built-in profiler to identify performance bottlenecks
- Monitor memory usage with the memory tracker
- Enable debug rendering for collision bounds
- Use performance monitor for real-time analysis
```

## Testing and Validation

### Example Project Tests

#### Hello World Test

```c
// tests/examples/test_examples.c
#include "playdate_engine.h"
#include <assert.h>
#include <stdio.h>

void test_hello_world_example(void) {
    printf("Testing Hello World example...\n");
    
    // Initialize engine
    engine_init(NULL);
    
    // Create basic scene
    Scene* scene = scene_create("Test", 10);
    assert(scene != NULL);
    
    // Create GameObject
    GameObject* obj = game_object_create(scene);
    assert(obj != NULL);
    
    // Basic functionality test
    game_object_set_position(obj, 100, 100);
    float x, y;
    game_object_get_position(obj, &x, &y);
    assert(x == 100.0f && y == 100.0f);
    
    // Cleanup
    scene_destroy(scene);
    
    printf("✓ Hello World example test passed\n");
}
```

#### Sprite Demo Test

```c
void test_sprite_demo_example(void) {
    printf("Testing Sprite Demo example...\n");
    
    component_registry_init();
    sprite_component_register();
    
    Scene* scene = scene_create("SpriteTest", 10);
    GameObject* obj = game_object_create(scene);
    
    // Add sprite component
    SpriteComponent* sprite = sprite_component_create(obj);
    assert(sprite != NULL);
    
    GameObjectResult result = game_object_add_component(obj, (Component*)sprite);
    assert(result == GAMEOBJECT_OK);
    
    // Test sprite properties
    sprite_component_set_anchor(sprite, 0.5f, 0.5f);
    float anchorX, anchorY;
    sprite_component_get_anchor(sprite, &anchorX, &anchorY);
    assert(anchorX == 0.5f && anchorY == 0.5f);
    
    // Cleanup
    scene_destroy(scene);
    component_registry_shutdown();
    
    printf("✓ Sprite Demo example test passed\n");
}
```

#### Performance Test

```c
void test_platformer_performance(void) {
    printf("Testing Platformer performance...\n");
    
    // This would test the platformer example under load
    // to ensure it meets performance targets
    
    profiler_init();
    profiler_enable(true);
    
    // Run platformer simulation
    // ... (simplified for brevity)
    
    // Verify performance
    float avgFrameTime = profiler_get_average_frame_time();
    assert(avgFrameTime < 33.0f); // 30 FPS target
    
    profiler_shutdown();
    
    printf("✓ Platformer performance test passed\n");
}
```

## Integration Points

### All Engine Systems
- Examples demonstrate integration of all engine components
- Real-world validation of API design and usability
- Performance testing under game development scenarios
- Documentation of best practices and common patterns

## Success Criteria

### Functional Requirements
- [ ] Complete set of examples from basic to advanced
- [ ] All examples run smoothly on Playdate hardware
- [ ] Comprehensive documentation and tutorials
- [ ] Project templates for rapid development
- [ ] Performance demonstrations meeting targets

### Educational Value
- [ ] Clear progression from simple to complex examples
- [ ] Well-commented code with explanations
- [ ] Best practices demonstrated throughout
- [ ] Common pitfalls and solutions documented
- [ ] Asset creation and optimization guidance

### Quality Requirements
- [ ] All examples tested and validated
- [ ] Performance benchmarks for complex games
- [ ] Cross-platform compatibility verified
- [ ] Complete API coverage in examples

## Next Steps

Upon completion of this phase:
1. Verify all examples run correctly on target hardware
2. Validate performance targets in real game scenarios
3. Gather feedback from developers using examples
4. Document lessons learned and update engine accordingly
5. Prepare for engine release with comprehensive example suite

This phase validates the engine's real-world applicability while providing developers with the resources needed to create compelling games efficiently.