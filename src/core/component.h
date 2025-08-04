#ifndef COMPONENT_H
#define COMPONENT_H

#include "memory_pool.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct GameObject GameObject;

// Component type enumeration (powers of 2 for bitmask)
typedef enum {
    COMPONENT_TYPE_NONE      = 0,
    COMPONENT_TYPE_TRANSFORM = 1 << 0,   // Always present, bit 0
    COMPONENT_TYPE_SPRITE    = 1 << 1,   // Rendering, bit 1
    COMPONENT_TYPE_COLLISION = 1 << 2,   // Physics, bit 2
    COMPONENT_TYPE_SCRIPT    = 1 << 3,   // Scripting, bit 3
    COMPONENT_TYPE_AUDIO     = 1 << 4,   // Audio, bit 4
    COMPONENT_TYPE_ANIMATION = 1 << 5,   // Animation, bit 5
    COMPONENT_TYPE_PARTICLES = 1 << 6,   // Particle systems, bit 6
    COMPONENT_TYPE_UI        = 1 << 7,   // UI elements, bit 7
    // Reserve bits 8-31 for future component types
    COMPONENT_TYPE_CUSTOM_BASE = 1 << 16 // Custom components start here
} ComponentType;

// Forward declare Component for VTable
typedef struct Component Component;

// Component lifecycle and behavior interface
typedef struct ComponentVTable {
    // Lifecycle
    void (*init)(Component* component, GameObject* gameObject);
    void (*destroy)(Component* component);
    Component* (*clone)(const Component* component);
    
    // Runtime
    void (*update)(Component* component, float deltaTime);
    void (*fixedUpdate)(Component* component, float fixedDeltaTime);
    void (*render)(Component* component);
    
    // Events
    void (*onEnabled)(Component* component);
    void (*onDisabled)(Component* component);
    void (*onGameObjectDestroyed)(Component* component);
    
    // Serialization (for save/load systems)
    uint32_t (*getSerializedSize)(const Component* component);
    bool (*serialize)(const Component* component, void* buffer, uint32_t bufferSize);
    bool (*deserialize)(Component* component, const void* buffer, uint32_t bufferSize);
} ComponentVTable;

// Base component structure (48 bytes, 16-byte aligned)
struct Component {
    ComponentType type;                // 4 bytes - component type bitmask
    uint32_t id;                       // 4 bytes - unique component ID
    const ComponentVTable* vtable;     // 8 bytes - virtual function table
    GameObject* gameObject;            // 8 bytes - owning game object
    uint8_t enabled;                   // 1 byte - active state
    uint8_t padding[23];               // 23 bytes - explicit padding for 48-byte alignment
};

// Component system results
typedef enum {
    COMPONENT_OK = 0,
    COMPONENT_ERROR_NULL_POINTER,
    COMPONENT_ERROR_INVALID_TYPE,
    COMPONENT_ERROR_ALREADY_EXISTS,
    COMPONENT_ERROR_NOT_FOUND,
    COMPONENT_ERROR_POOL_FULL,
    COMPONENT_ERROR_VTABLE_NULL
} ComponentResult;

// Core component operations
ComponentResult component_init(Component* component, ComponentType type, 
                              const ComponentVTable* vtable, GameObject* gameObject);
void component_destroy(Component* component);

// Component state management
void component_set_enabled(Component* component, bool enabled);
bool component_is_enabled(const Component* component);

// Type checking utilities
bool component_is_type(const Component* component, ComponentType type);
const char* component_type_to_string(ComponentType type);

// Virtual function call helpers (with null checks)
void component_call_update(Component* component, float deltaTime);
void component_call_render(Component* component);
void component_call_on_enabled(Component* component);
void component_call_on_disabled(Component* component);

#endif // COMPONENT_H