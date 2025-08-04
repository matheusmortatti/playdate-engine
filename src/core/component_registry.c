#include "component_registry.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Global component registry instance
static ComponentRegistry g_componentRegistry = {0};

ComponentResult component_registry_init(void) {
    memset(&g_componentRegistry, 0, sizeof(ComponentRegistry));
    g_componentRegistry.nextComponentId = 1; // Start from 1 (0 is invalid)
    return COMPONENT_OK;
}

void component_registry_shutdown(void) {
    // Destroy all pools
    for (uint32_t i = 0; i < MAX_COMPONENT_TYPES; i++) {
        ComponentTypeInfo* info = &g_componentRegistry.typeInfo[i];
        if (info->registered) {
            object_pool_destroy(&info->pool);
            info->registered = false;
        }
    }
    
    memset(&g_componentRegistry, 0, sizeof(ComponentRegistry));
}

// Helper function to get bit position from component type
static uint32_t get_bit_position(ComponentType type) {
    if (type == 0) return MAX_COMPONENT_TYPES; // Invalid
    
    uint32_t bitPosition = 0;
    uint32_t typeBit = type;
    
    // Find the position of the first (and should be only) set bit
    while (typeBit > 1) {
        typeBit >>= 1;
        bitPosition++;
    }
    
    // Verify it's a power of 2 (only one bit set)
    if ((type & (type - 1)) != 0) {
        return MAX_COMPONENT_TYPES; // Invalid - not a power of 2
    }
    
    return bitPosition;
}

ComponentResult component_registry_register_type(ComponentType type, 
                                                uint32_t componentSize,
                                                uint32_t poolCapacity,
                                                const ComponentVTable* defaultVTable,
                                                const char* typeName) {
    if (!defaultVTable || !typeName) {
        return COMPONENT_ERROR_NULL_POINTER;
    }
    
    uint32_t bitPosition = get_bit_position(type);
    if (bitPosition >= MAX_COMPONENT_TYPES) {
        return COMPONENT_ERROR_INVALID_TYPE;
    }
    
    ComponentTypeInfo* info = &g_componentRegistry.typeInfo[bitPosition];
    
    if (info->registered) {
        return COMPONENT_ERROR_ALREADY_EXISTS;
    }
    
    // Ensure minimum component size for alignment
    uint32_t alignedSize = componentSize < sizeof(Component) ? sizeof(Component) : componentSize;
    alignedSize = ALIGN_SIZE(alignedSize);
    
    // Initialize object pool
    char poolName[64];
    snprintf(poolName, sizeof(poolName), "ComponentPool_%s", typeName);
    
    PoolResult poolResult = object_pool_init(&info->pool, alignedSize, poolCapacity, poolName);
    if (poolResult != POOL_OK) {
        return COMPONENT_ERROR_POOL_FULL;
    }
    
    // Set up type info
    info->type = type;
    info->componentSize = alignedSize;
    info->poolCapacity = poolCapacity;
    info->defaultVTable = defaultVTable;
    info->typeName = typeName;
    info->registered = true;
    
    g_componentRegistry.registeredTypeCount++;
    
    return COMPONENT_OK;
}

Component* component_registry_create(ComponentType type, GameObject* gameObject) {
    if (!gameObject) {
        return NULL;
    }
    
    uint32_t bitPosition = get_bit_position(type);
    if (bitPosition >= MAX_COMPONENT_TYPES) {
        return NULL;
    }
    
    ComponentTypeInfo* info = &g_componentRegistry.typeInfo[bitPosition];
    if (!info->registered) {
        return NULL;
    }
    
    // Allocate component from pool
    Component* component = (Component*)object_pool_alloc(&info->pool);
    if (!component) {
        return NULL;
    }
    
    // Initialize component
    ComponentResult result = component_init(component, type, info->defaultVTable, gameObject);
    if (result != COMPONENT_OK) {
        object_pool_free(&info->pool, component);
        return NULL;
    }
    
    component->id = g_componentRegistry.nextComponentId++;
    
    // Call initialization vtable function
    if (component->vtable && component->vtable->init) {
        component->vtable->init(component, gameObject);
    }
    
    return component;
}

ComponentResult component_registry_destroy(Component* component) {
    if (!component) {
        return COMPONENT_ERROR_NULL_POINTER;
    }
    
    // Call destruction vtable function
    if (component->vtable && component->vtable->destroy) {
        component->vtable->destroy(component);
    }
    
    // Find the appropriate pool
    uint32_t bitPosition = get_bit_position(component->type);
    if (bitPosition >= MAX_COMPONENT_TYPES) {
        return COMPONENT_ERROR_INVALID_TYPE;
    }
    
    ComponentTypeInfo* info = &g_componentRegistry.typeInfo[bitPosition];
    if (!info->registered) {
        return COMPONENT_ERROR_NOT_FOUND;
    }
    
    // Return to pool first, then clear
    PoolResult poolResult = object_pool_free(&info->pool, component);
    if (poolResult == POOL_OK) {
        // Clear the component after successful pool return
        component_destroy(component);
        return COMPONENT_OK;
    } else {
        return COMPONENT_ERROR_POOL_FULL;
    }
}

bool component_registry_is_type_registered(ComponentType type) {
    uint32_t bitPosition = get_bit_position(type);
    if (bitPosition >= MAX_COMPONENT_TYPES) {
        return false;
    }
    
    return g_componentRegistry.typeInfo[bitPosition].registered;
}

const ComponentTypeInfo* component_registry_get_type_info(ComponentType type) {
    uint32_t bitPosition = get_bit_position(type);
    if (bitPosition >= MAX_COMPONENT_TYPES) {
        return NULL;
    }
    
    ComponentTypeInfo* info = &g_componentRegistry.typeInfo[bitPosition];
    return info->registered ? info : NULL;
}

uint32_t component_registry_get_component_count(ComponentType type) {
    uint32_t bitPosition = get_bit_position(type);
    if (bitPosition >= MAX_COMPONENT_TYPES) {
        return 0;
    }
    
    ComponentTypeInfo* info = &g_componentRegistry.typeInfo[bitPosition];
    if (!info->registered) {
        return 0;
    }
    
    return info->poolCapacity - object_pool_get_free_count(&info->pool);
}

ObjectPool* component_registry_get_pool(ComponentType type) {
    uint32_t bitPosition = get_bit_position(type);
    if (bitPosition >= MAX_COMPONENT_TYPES) {
        return NULL;
    }
    
    ComponentTypeInfo* info = &g_componentRegistry.typeInfo[bitPosition];
    return info->registered ? &info->pool : NULL;
}

void component_registry_print_stats(void) {
    printf("=== Component Registry Statistics ===\n");
    printf("Registered types: %u/%u\n", g_componentRegistry.registeredTypeCount, MAX_COMPONENT_TYPES);
    printf("Next component ID: %u\n", g_componentRegistry.nextComponentId);
    
    for (uint32_t i = 0; i < MAX_COMPONENT_TYPES; i++) {
        ComponentTypeInfo* info = &g_componentRegistry.typeInfo[i];
        if (info->registered) {
            uint32_t used = info->poolCapacity - object_pool_get_free_count(&info->pool);
            float usage = (float)used / info->poolCapacity * 100.0f;
            
            printf("  %s: %u/%u components (%.1f%%) - %u bytes each\n",
                   info->typeName, used, info->poolCapacity, usage, info->componentSize);
        }
    }
    
    printf("Total memory usage: %u bytes\n", component_registry_get_total_memory_usage());
    printf("=====================================\n");
}

uint32_t component_registry_get_total_memory_usage(void) {
    uint32_t totalMemory = sizeof(ComponentRegistry);
    
    for (uint32_t i = 0; i < MAX_COMPONENT_TYPES; i++) {
        ComponentTypeInfo* info = &g_componentRegistry.typeInfo[i];
        if (info->registered) {
            // Pool memory usage = component size * capacity + pool overhead
            totalMemory += info->componentSize * info->poolCapacity;
            totalMemory += info->poolCapacity * sizeof(uint32_t); // Free list
            totalMemory += info->poolCapacity * sizeof(uint8_t);  // Object states
        }
    }
    
    return totalMemory;
}