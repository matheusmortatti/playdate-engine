#ifndef COMPONENT_FACTORY_H
#define COMPONENT_FACTORY_H

#include "../core/component.h"
#include "../core/component_registry.h"

// Component factory initialization
ComponentResult component_factory_init(void);
void component_factory_shutdown(void);

// Generic component creation
Component* component_factory_create(ComponentType type, GameObject* gameObject);
ComponentResult component_factory_destroy(Component* component);

// Typed component creation helpers
struct TransformComponent* component_factory_create_transform(GameObject* gameObject);
// Additional typed creators will be added as more components are implemented

// Batch operations
ComponentResult component_factory_register_all_types(void);
uint32_t component_factory_get_registered_type_count(void);

// Factory stats and debugging
void component_factory_print_stats(void);
ComponentResult component_factory_validate_all_pools(void);

#endif // COMPONENT_FACTORY_H