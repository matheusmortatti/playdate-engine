#include "component_factory.h"
#include "transform_component.h"
#include <stdio.h>

// Internal mock for registration
typedef struct MockGameObject {
    uint32_t id;
    const char* name;
} MockGameObject;

ComponentResult component_factory_init(void) {
    // Initialize the component registry
    return component_registry_init();
}

void component_factory_shutdown(void) {
    component_registry_shutdown();
}

Component* component_factory_create(ComponentType type, GameObject* gameObject) {
    // Ensure the requested type is registered
    if (!component_registry_is_type_registered(type) && type == COMPONENT_TYPE_TRANSFORM) {
        // Auto-register transform component if not already registered
        component_factory_register_all_types();
    }
    
    return component_registry_create(type, gameObject);
}

ComponentResult component_factory_destroy(Component* component) {
    return component_registry_destroy(component);
}

struct TransformComponent* component_factory_create_transform(GameObject* gameObject) {
    return transform_component_create(gameObject);
}

ComponentResult component_factory_register_all_types(void) {
    // We'll register basic component types with default sizes
    // For now, just register transform since it's the only one implemented
    
    // Transform will auto-register when created, but we can create and destroy
    // a dummy object to trigger registration
    MockGameObject dummy = {999, "FactoryDummy"};
    TransformComponent* temp = transform_component_create((GameObject*)&dummy);
    if (temp) {
        transform_component_destroy(temp);
    }
    
    return COMPONENT_OK;
}

uint32_t component_factory_get_registered_type_count(void) {
    uint32_t count = 0;
    
    // Count registered types by checking each possible type
    ComponentType types[] = {
        COMPONENT_TYPE_TRANSFORM,
        COMPONENT_TYPE_SPRITE,
        COMPONENT_TYPE_COLLISION,
        COMPONENT_TYPE_SCRIPT,
        COMPONENT_TYPE_AUDIO,
        COMPONENT_TYPE_ANIMATION,
        COMPONENT_TYPE_PARTICLES,
        COMPONENT_TYPE_UI
    };
    
    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        if (component_registry_is_type_registered(types[i])) {
            count++;
        }
    }
    
    return count;
}

void component_factory_print_stats(void) {
    printf("=== Component Factory Statistics ===\n");
    printf("Registered component types: %u\n", component_factory_get_registered_type_count());
    
    // Delegate to registry for detailed stats
    component_registry_print_stats();
}

ComponentResult component_factory_validate_all_pools(void) {
    // Check if we have any registered types
    if (component_factory_get_registered_type_count() == 0) {
        return COMPONENT_ERROR_NOT_FOUND;
    }
    
    // For now, assume pools are valid if registry is initialized
    // More sophisticated validation could be added here
    return COMPONENT_OK;
}