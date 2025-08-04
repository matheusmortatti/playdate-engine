#ifndef COMPONENT_REGISTRY_H
#define COMPONENT_REGISTRY_H

#include "component.h"
#include "memory_pool.h"

#define MAX_COMPONENT_TYPES 32
#define DEFAULT_COMPONENT_POOL_SIZE 1000

// Component type registration info
typedef struct ComponentTypeInfo {
    ComponentType type;
    uint32_t componentSize;
    uint32_t poolCapacity;
    ObjectPool pool;
    const ComponentVTable* defaultVTable;
    const char* typeName;
    bool registered;
} ComponentTypeInfo;

// Global component registry
typedef struct ComponentRegistry {
    ComponentTypeInfo typeInfo[MAX_COMPONENT_TYPES];
    uint32_t registeredTypeCount;
    uint32_t nextComponentId;
} ComponentRegistry;

// Registry management
ComponentResult component_registry_init(void);
void component_registry_shutdown(void);

// Type registration
ComponentResult component_registry_register_type(ComponentType type, 
                                                uint32_t componentSize,
                                                uint32_t poolCapacity,
                                                const ComponentVTable* defaultVTable,
                                                const char* typeName);

// Component creation and destruction
Component* component_registry_create(ComponentType type, GameObject* gameObject);
ComponentResult component_registry_destroy(Component* component);

// Registry queries
bool component_registry_is_type_registered(ComponentType type);
const ComponentTypeInfo* component_registry_get_type_info(ComponentType type);
uint32_t component_registry_get_component_count(ComponentType type);
ObjectPool* component_registry_get_pool(ComponentType type);

// Debug and profiling
void component_registry_print_stats(void);
uint32_t component_registry_get_total_memory_usage(void);

#endif // COMPONENT_REGISTRY_H