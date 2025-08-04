# Phase 8: Lua Bindings

## Objective

Implement comprehensive Lua bindings for the C engine core, enabling rapid game development with scripting while maintaining high performance through efficient C-to-Lua bridges. This system provides a complete Lua API for all engine components with minimal runtime overhead.

## Prerequisites

- **Phase 1-7**: Complete C engine implementation
- Lua 5.4+ integration knowledge
- Understanding of C-Lua interop patterns
- Knowledge of userdata and metatable systems

## Technical Specifications

### Performance Targets
- **Binding overhead**: < 10ns per function call
- **Memory efficiency**: < 20% overhead for Lua objects
- **Garbage collection**: Minimal impact on frame rate
- **C-Lua transitions**: < 5μs for complex operations

### Lua Integration Goals
- **Complete API coverage**: All C functionality accessible from Lua
- **Type safety**: Runtime validation and error handling
- **Memory management**: Automatic cleanup and reference counting
- **Performance**: Hot-path operations remain in C
- **Debugging**: Comprehensive error reporting and stack traces

## Code Structure

```
src/bindings/
├── lua_engine.h/.c          # Core Lua state management
├── lua_gameobject.h/.c      # GameObject Lua interface
├── lua_components.h/.c      # Component system bindings
├── lua_scene.h/.c           # Scene management bindings
├── lua_math.h/.c            # Math and utility bindings
└── lua_debug.h/.c           # Debugging and profiling

lua/
├── engine.lua               # High-level Lua API
├── gameobject.lua           # GameObject helper functions
├── components/              # Component Lua modules
│   ├── transform.lua
│   ├── sprite.lua
│   └── collision.lua
└── utils/                   # Utility modules
    ├── math.lua
    ├── input.lua
    └── debug.lua

tests/bindings/
├── test_lua_engine.c        # Core binding tests
├── test_lua_gameobject.c    # GameObject binding tests
├── test_lua_components.c    # Component binding tests
└── test_lua_perf.c          # Performance benchmarks

examples/lua/
├── basic_game.lua           # Simple Lua game
├── platformer.lua           # Complete platformer example
└── scripted_components.lua  # Custom Lua components
```

## Implementation Steps

### Step 1: Core Lua Engine Integration

#### Core Header and Definitions

```c
// lua_engine.h
#ifndef LUA_ENGINE_H
#define LUA_ENGINE_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "scene.h"
#include "pd_api.h"
#include <stdint.h>
#include <stdbool.h>

#define LUA_ENGINE_VERSION "1.0.0"
#define MAX_LUA_STACK_SIZE 1000
#define MAX_ERROR_MESSAGE_SIZE 512
```

#### Lua Engine Structure

```c
// Lua engine state
typedef struct LuaEngine {
    lua_State* L;                    // Main Lua state
    PlaydateAPI* pd;                 // Playdate API reference
    Scene* activeScene;              // Current scene
    
    // Error handling
    char lastError[MAX_ERROR_MESSAGE_SIZE];
    bool hasError;
    
    // Performance tracking
    uint32_t luaCallCount;
    uint32_t cCallCount;
    float luaExecutionTime;
    
    // Memory management
    size_t luaMemoryUsage;
    size_t maxLuaMemory;
    
    // Configuration
    bool enableDebugHooks;
    bool enableGCMetrics;
    
} LuaEngine;
```

#### Engine Management Functions

```c
// Engine management
LuaEngine* lua_engine_create(PlaydateAPI* pd);
void lua_engine_destroy(LuaEngine* engine);
bool lua_engine_initialize(LuaEngine* engine);

// Script execution
bool lua_engine_load_file(LuaEngine* engine, const char* filename);
bool lua_engine_load_string(LuaEngine* engine, const char* script);
bool lua_engine_call_function(LuaEngine* engine, const char* functionName, 
                             int numArgs, int numResults);
```

#### Error and Memory Management

```c
// Error handling
const char* lua_engine_get_last_error(LuaEngine* engine);
void lua_engine_clear_error(LuaEngine* engine);
bool lua_engine_has_error(LuaEngine* engine);

// Memory management
void lua_engine_collect_garbage(LuaEngine* engine);
size_t lua_engine_get_memory_usage(LuaEngine* engine);
void lua_engine_set_memory_limit(LuaEngine* engine, size_t maxMemory);
```

#### Binding Registration

```c
// Binding registration
void lua_engine_register_all_bindings(LuaEngine* engine);
void lua_engine_register_gameobject_bindings(LuaEngine* engine);
void lua_engine_register_component_bindings(LuaEngine* engine);
void lua_engine_register_scene_bindings(LuaEngine* engine);
void lua_engine_register_math_bindings(LuaEngine* engine);
```

#### Utility and Debugging Functions

```c
// Utility functions
void lua_engine_push_gameobject(LuaEngine* engine, GameObject* gameObject);
GameObject* lua_engine_check_gameobject(LuaEngine* engine, int index);
void lua_engine_push_component(LuaEngine* engine, Component* component);
Component* lua_engine_check_component(LuaEngine* engine, int index, 
                                     ComponentType type);

// Performance and debugging
void lua_engine_print_stats(LuaEngine* engine);
void lua_engine_reset_stats(LuaEngine* engine);
void lua_engine_enable_debug_hooks(LuaEngine* engine, bool enabled);

// Global engine access
LuaEngine* lua_engine_get_global(void);
void lua_engine_set_global(LuaEngine* engine);

#endif // LUA_ENGINE_H
```

### Step 2: GameObject Lua Bindings

#### GameObject Wrapper Structure

```c
// lua_gameobject.c
#include "lua_gameobject.h"
#include "lua_engine.h"
#include "game_object.h"
#include "transform_component.h"

#define GAMEOBJECT_METATABLE "PlaydateEngine.GameObject"

// GameObject userdata wrapper
typedef struct LuaGameObject {
    GameObject* gameObject;
    Scene* scene;                // For validation
    bool isValid;               // Prevents use after destruction
} LuaGameObject;
```

#### GameObject Creation and Destruction

```c
// GameObject creation
static int lua_gameobject_new(lua_State* L) {
    LuaEngine* engine = lua_engine_get_global();
    
    // Get scene (optional parameter)
    Scene* scene = engine->activeScene;
    if (lua_gettop(L) > 0 && !lua_isnil(L, 1)) {
        // TODO: Check if user provided a scene
    }
    
    // Create GameObject
    GameObject* gameObject = game_object_create(scene);
    if (!gameObject) {
        return luaL_error(L, "Failed to create GameObject");
    }
    
    // Create Lua wrapper
    LuaGameObject* luaObj = (LuaGameObject*)lua_newuserdata(L, sizeof(LuaGameObject));
    luaObj->gameObject = gameObject;
    luaObj->scene = scene;
    luaObj->isValid = true;
    
    // Set metatable
    luaL_getmetatable(L, GAMEOBJECT_METATABLE);
    lua_setmetatable(L, -2);
    
    return 1;
}

// GameObject destruction
static int lua_gameobject_destroy(lua_State* L) {
    LuaGameObject* luaObj = (LuaGameObject*)luaL_checkudata(L, 1, GAMEOBJECT_METATABLE);
    
    if (luaObj->isValid && luaObj->gameObject) {
        game_object_destroy(luaObj->gameObject);
        luaObj->gameObject = NULL;
        luaObj->isValid = false;
    }
    
    return 0;
}
```

#### Position Management

```c
// GameObject position
static int lua_gameobject_set_position(lua_State* L) {
    LuaGameObject* luaObj = (LuaGameObject*)luaL_checkudata(L, 1, GAMEOBJECT_METATABLE);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    
    if (!luaObj->isValid) {
        return luaL_error(L, "Attempt to use destroyed GameObject");
    }
    
    game_object_set_position(luaObj->gameObject, x, y);
    return 0;
}

static int lua_gameobject_get_position(lua_State* L) {
    LuaGameObject* luaObj = (LuaGameObject*)luaL_checkudata(L, 1, GAMEOBJECT_METATABLE);
    
    if (!luaObj->isValid) {
        return luaL_error(L, "Attempt to use destroyed GameObject");
    }
    
    float x, y;
    game_object_get_position(luaObj->gameObject, &x, &y);
    
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}
```

#### Component Management

```c
// Component management
static int lua_gameobject_add_component(lua_State* L) {
    LuaGameObject* luaObj = (LuaGameObject*)luaL_checkudata(L, 1, GAMEOBJECT_METATABLE);
    const char* componentType = luaL_checkstring(L, 2);
    
    if (!luaObj->isValid) {
        return luaL_error(L, "Attempt to use destroyed GameObject");
    }
    
    ComponentType type = COMPONENT_TYPE_NONE;
    if (strcmp(componentType, "Sprite") == 0) {
        type = COMPONENT_TYPE_SPRITE;
    } else if (strcmp(componentType, "Collision") == 0) {
        type = COMPONENT_TYPE_COLLISION;
    } else {
        return luaL_error(L, "Unknown component type: %s", componentType);
    }
    
    Component* component = component_registry_create(type, luaObj->gameObject);
    if (!component) {
        return luaL_error(L, "Failed to create component: %s", componentType);
    }
    
    GameObjectResult result = game_object_add_component(luaObj->gameObject, component);
    if (result != GAMEOBJECT_OK) {
        component_registry_destroy(component);
        return luaL_error(L, "Failed to add component to GameObject");
    }
    
    // Push component as Lua object
    lua_engine_push_component(lua_engine_get_global(), component);
    return 1;
}

static int lua_gameobject_get_component(lua_State* L) {
    LuaGameObject* luaObj = (LuaGameObject*)luaL_checkudata(L, 1, GAMEOBJECT_METATABLE);
    const char* componentType = luaL_checkstring(L, 2);
    
    if (!luaObj->isValid) {
        return luaL_error(L, "Attempt to use destroyed GameObject");
    }
    
    ComponentType type = COMPONENT_TYPE_NONE;
    if (strcmp(componentType, "Transform") == 0) {
        type = COMPONENT_TYPE_TRANSFORM;
    } else if (strcmp(componentType, "Sprite") == 0) {
        type = COMPONENT_TYPE_SPRITE;
    } else if (strcmp(componentType, "Collision") == 0) {
        type = COMPONENT_TYPE_COLLISION;
    } else {
        return luaL_error(L, "Unknown component type: %s", componentType);
    }
    
    Component* component = game_object_get_component(luaObj->gameObject, type);
    if (!component) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_engine_push_component(lua_engine_get_global(), component);
    return 1;
}
```

#### Binding Registration

```c
// GameObject metatable
static const luaL_Reg gameobject_methods[] = {
    {"new", lua_gameobject_new},
    {"destroy", lua_gameobject_destroy},
    {"setPosition", lua_gameobject_set_position},
    {"getPosition", lua_gameobject_get_position},
    {"addComponent", lua_gameobject_add_component},
    {"getComponent", lua_gameobject_get_component},
    {"__gc", lua_gameobject_destroy},
    {NULL, NULL}
};

void lua_engine_register_gameobject_bindings(LuaEngine* engine) {
    lua_State* L = engine->L;
    
    // Create metatable
    luaL_newmetatable(L, GAMEOBJECT_METATABLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, gameobject_methods, 0);
    lua_pop(L, 1);
    
    // Create GameObject table
    lua_newtable(L);
    lua_pushcfunction(L, lua_gameobject_new);
    lua_setfield(L, -2, "new");
    lua_setglobal(L, "GameObject");
}
```

#### Utility Functions

```c
// Utility functions
void lua_engine_push_gameobject(LuaEngine* engine, GameObject* gameObject) {
    lua_State* L = engine->L;
    
    if (!gameObject) {
        lua_pushnil(L);
        return;
    }
    
    LuaGameObject* luaObj = (LuaGameObject*)lua_newuserdata(L, sizeof(LuaGameObject));
    luaObj->gameObject = gameObject;
    luaObj->scene = game_object_get_scene(gameObject);
    luaObj->isValid = true;
    
    luaL_getmetatable(L, GAMEOBJECT_METATABLE);
    lua_setmetatable(L, -2);
}

GameObject* lua_engine_check_gameobject(LuaEngine* engine, int index) {
    LuaGameObject* luaObj = (LuaGameObject*)luaL_checkudata(engine->L, index, GAMEOBJECT_METATABLE);
    
    if (!luaObj->isValid) {
        luaL_error(engine->L, "Attempt to use destroyed GameObject");
        return NULL;
    }
    
    return luaObj->gameObject;
}
```

### Step 3: High-Level Lua API

#### Core Engine Module

```lua
-- engine.lua
local Engine = {}

-- Core engine functions
function Engine.init()
    -- Initialize engine systems
    print("Playdate Engine Lua API initialized")
end

function Engine.update(dt)
    -- Update all game systems
    -- This would call into C for performance-critical updates
end

function Engine.render()
    -- Render all game objects
    -- This would call into C for actual rendering
end
```

#### GameObject Helper Functions

```lua
-- GameObject helper functions
function Engine.createGameObject(x, y)
    local obj = GameObject.new()
    if x and y then
        obj:setPosition(x, y)
    end
    return obj
end

function Engine.createSprite(imagePath, x, y)
    local obj = Engine.createGameObject(x, y)
    local sprite = obj:addComponent("Sprite")
    if imagePath then
        sprite:loadImage(imagePath)
    end
    return obj, sprite
end

-- Scene management helpers
function Engine.loadScene(sceneName)
    -- Load scene from file or create new scene
    print("Loading scene: " .. sceneName)
end
```

#### Input Handling Module

```lua
-- Input handling
local Input = {}

function Input.isPressed(button)
    -- Check if button is currently pressed
    return false -- Placeholder
end

function Input.wasPressed(button)
    -- Check if button was just pressed this frame
    return false -- Placeholder
end

function Input.wasReleased(button)
    -- Check if button was just released this frame
    return false -- Placeholder
end
```

#### Math Utilities

```lua
-- Math utilities
local Math = {}

function Math.clamp(value, min, max)
    return math.max(min, math.min(max, value))
end

function Math.lerp(a, b, t)
    return a + (b - a) * t
end

function Math.distance(x1, y1, x2, y2)
    local dx = x2 - x1
    local dy = y2 - y1
    return math.sqrt(dx * dx + dy * dy)
end

-- Export modules
Engine.Input = Input
Engine.Math = Math

return Engine
```

## Unit Tests

### Lua Binding Tests

#### Engine Tests

```c
// tests/bindings/test_lua_engine.c
#include "lua_engine.h"
#include <assert.h>
#include <stdio.h>

void test_lua_engine_creation(void) {
    LuaEngine* engine = lua_engine_create(NULL);
    assert(engine != NULL);
    assert(engine->L != NULL);
    assert(!lua_engine_has_error(engine));
    
    bool result = lua_engine_initialize(engine);
    assert(result == true);
    
    lua_engine_destroy(engine);
    printf("✓ Lua engine creation test passed\n");
}

void test_lua_script_execution(void) {
    LuaEngine* engine = lua_engine_create(NULL);
    lua_engine_initialize(engine);
    
    // Test simple script execution
    const char* script = "return 2 + 3";
    bool result = lua_engine_load_string(engine, script);
    assert(result == true);
    
    // Call the loaded function
    result = lua_engine_call_function(engine, NULL, 0, 1);
    assert(result == true);
    
    // Check result
    lua_State* L = engine->L;
    assert(lua_isnumber(L, -1));
    assert(lua_tonumber(L, -1) == 5);
    
    lua_engine_destroy(engine);
    printf("✓ Lua script execution test passed\n");
}
```

#### Error Handling Tests

```c
void test_lua_error_handling(void) {
    LuaEngine* engine = lua_engine_create(NULL);
    lua_engine_initialize(engine);
    
    // Test script with syntax error
    const char* badScript = "invalid lua syntax {{{";
    bool result = lua_engine_load_string(engine, badScript);
    assert(result == false);
    assert(lua_engine_has_error(engine));
    
    const char* error = lua_engine_get_last_error(engine);
    assert(error != NULL);
    assert(strlen(error) > 0);
    
    lua_engine_clear_error(engine);
    assert(!lua_engine_has_error(engine));
    
    lua_engine_destroy(engine);
    printf("✓ Lua error handling test passed\n");
}
```

### GameObject Binding Tests

#### Basic GameObject Tests

```c
// tests/bindings/test_lua_gameobject.c
#include "lua_engine.h"
#include "lua_gameobject.h"
#include <assert.h>
#include <stdio.h>

void test_lua_gameobject_creation(void) {
    component_registry_init();
    transform_component_register();
    
    LuaEngine* engine = lua_engine_create(NULL);
    lua_engine_initialize(engine);
    lua_engine_register_gameobject_bindings(engine);
    
    // Test GameObject creation from Lua
    const char* script = 
        "local obj = GameObject.new()\n"
        "return obj";
    
    bool result = lua_engine_load_string(engine, script);
    assert(result == true);
    
    result = lua_engine_call_function(engine, NULL, 0, 1);
    assert(result == true);
    
    // Verify we got a GameObject
    GameObject* obj = lua_engine_check_gameobject(engine, -1);
    assert(obj != NULL);
    
    lua_engine_destroy(engine);
    component_registry_shutdown();
    printf("✓ Lua GameObject creation test passed\n");
}
```

#### Position Management Tests

```c
void test_lua_gameobject_position(void) {
    component_registry_init();
    transform_component_register();
    
    LuaEngine* engine = lua_engine_create(NULL);
    lua_engine_initialize(engine);
    lua_engine_register_gameobject_bindings(engine);
    
    // Test position setting and getting
    const char* script = 
        "local obj = GameObject.new()\n"
        "obj:setPosition(100, 200)\n"
        "local x, y = obj:getPosition()\n"
        "return x, y";
    
    bool result = lua_engine_load_string(engine, script);
    assert(result == true);
    
    result = lua_engine_call_function(engine, NULL, 0, 2);
    assert(result == true);
    
    lua_State* L = engine->L;
    assert(lua_isnumber(L, -2));
    assert(lua_isnumber(L, -1));
    assert(lua_tonumber(L, -2) == 100);
    assert(lua_tonumber(L, -1) == 200);
    
    lua_engine_destroy(engine);
    component_registry_shutdown();
    printf("✓ Lua GameObject position test passed\n");
}
```

#### Component Management Tests

```c
void test_lua_component_management(void) {
    component_registry_init();
    transform_component_register();
    sprite_component_register();
    
    LuaEngine* engine = lua_engine_create(NULL);
    lua_engine_initialize(engine);
    lua_engine_register_all_bindings(engine);
    
    // Test component addition and retrieval
    const char* script = 
        "local obj = GameObject.new()\n"
        "local sprite = obj:addComponent('Sprite')\n"
        "local transform = obj:getComponent('Transform')\n"
        "return sprite ~= nil, transform ~= nil";
    
    bool result = lua_engine_load_string(engine, script);
    assert(result == true);
    
    result = lua_engine_call_function(engine, NULL, 0, 2);
    assert(result == true);
    
    lua_State* L = engine->L;
    assert(lua_toboolean(L, -2) == 1); // sprite ~= nil
    assert(lua_toboolean(L, -1) == 1); // transform ~= nil
    
    lua_engine_destroy(engine);
    component_registry_shutdown();
    printf("✓ Lua component management test passed\n");
}
```

### Performance Tests

#### Function Call Benchmarks

```c
// tests/bindings/test_lua_perf.c
#include "lua_engine.h"
#include <time.h>
#include <stdio.h>

void benchmark_lua_function_calls(void) {
    LuaEngine* engine = lua_engine_create(NULL);
    lua_engine_initialize(engine);
    lua_engine_register_all_bindings(engine);
    
    // Create test script
    const char* script = 
        "function testFunction()\n"
        "  return 42\n"
        "end";
    
    lua_engine_load_string(engine, script);
    lua_engine_call_function(engine, NULL, 0, 0);
    
    clock_t start = clock();
    
    // Call Lua function many times
    for (int i = 0; i < 10000; i++) {
        lua_engine_call_function(engine, "testFunction", 0, 1);
        lua_pop(engine->L, 1); // Pop result
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000; // microseconds
    double per_call = time_taken / 10000;
    
    printf("Lua function calls: %.2f μs for 10,000 calls (%.2f μs per call)\n", 
           time_taken, per_call);
    
    // Verify performance target
    assert(per_call < 10); // Less than 10μs per call
    
    lua_engine_destroy(engine);
    printf("✓ Lua function call performance test passed\n");
}
```

#### GameObject Creation Benchmarks

```c
void benchmark_gameobject_creation(void) {
    component_registry_init();
    transform_component_register();
    
    LuaEngine* engine = lua_engine_create(NULL);
    lua_engine_initialize(engine);
    lua_engine_register_all_bindings(engine);
    
    const char* script = 
        "function createGameObject()\n"
        "  local obj = GameObject.new()\n"
        "  obj:setPosition(100, 200)\n"
        "  return obj\n"
        "end";
    
    lua_engine_load_string(engine, script);
    lua_engine_call_function(engine, NULL, 0, 0);
    
    clock_t start = clock();
    
    // Create GameObjects from Lua
    for (int i = 0; i < 1000; i++) {
        lua_engine_call_function(engine, "createGameObject", 0, 1);
        lua_pop(engine->L, 1); // Pop result
    }
    
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000; // milliseconds
    double per_object = time_taken / 1000;
    
    printf("GameObject creation: %.2f ms for 1,000 objects (%.2f ms per object)\n", 
           time_taken, per_object);
    
    // Verify reasonable performance
    assert(per_object < 1); // Less than 1ms per GameObject creation
    
    lua_engine_destroy(engine);
    component_registry_shutdown();
    printf("✓ GameObject creation performance test passed\n");
}
```

## Integration Points

### All Previous Phases
- Complete Lua API coverage for all C engine functionality
- Memory management integration with Lua garbage collector
- Performance optimization through selective C/Lua usage
- Error handling and debugging integration

## Performance Targets

### Binding Performance
- **Function call overhead**: < 10μs per Lua-to-C call
- **Memory overhead**: < 20% for Lua object wrappers
- **Garbage collection**: < 1ms impact per frame
- **Hot-path performance**: Critical code remains in C

## Testing Criteria

### Unit Test Requirements
- ✅ Lua engine initialization and cleanup
- ✅ Script loading and execution
- ✅ Error handling and reporting
- ✅ GameObject binding functionality
- ✅ Component system integration
- ✅ Memory management and garbage collection

### Performance Test Requirements
- ✅ Function call overhead benchmarks
- ✅ GameObject creation performance
- ✅ Memory usage measurements
- ✅ Garbage collection impact analysis

### Integration Test Requirements
- ✅ Complete API coverage validation
- ✅ C-Lua memory management
- ✅ Error propagation between C and Lua
- ✅ Performance optimization effectiveness

## Success Criteria

### Functional Requirements
- [ ] Complete Lua API for all engine functionality
- [ ] Type-safe bindings with runtime validation
- [ ] Automatic memory management and cleanup
- [ ] Comprehensive error handling and reporting
- [ ] High-level Lua utilities and helpers

### Performance Requirements
- [ ] < 10μs overhead per function call
- [ ] < 20% memory overhead for Lua objects
- [ ] Minimal garbage collection impact
- [ ] Performance-critical paths remain in C

### Quality Requirements
- [ ] 100% unit test coverage for binding system
- [ ] Performance benchmarks meet all targets
- [ ] Comprehensive documentation and examples
- [ ] Robust error handling and debugging support

## Next Steps

Upon completion of this phase:
1. Verify all Lua binding tests pass
2. Confirm performance benchmarks meet targets
3. Test complete API coverage and functionality
4. Proceed to Phase 9: Build System implementation
5. Begin implementing CMake and Playdate SDK integration

This phase provides the scripting foundation that enables rapid game development while maintaining engine performance through efficient C-Lua integration.