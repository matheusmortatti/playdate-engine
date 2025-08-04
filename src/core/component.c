#include "component.h"
#include <string.h>
#include <stdio.h>

ComponentResult component_init(Component* component, ComponentType type, 
                              const ComponentVTable* vtable, GameObject* gameObject) {
    if (!component || !vtable || !gameObject) {
        return COMPONENT_ERROR_NULL_POINTER;
    }
    
    if (type == COMPONENT_TYPE_NONE) {
        return COMPONENT_ERROR_INVALID_TYPE;
    }
    
    memset(component, 0, sizeof(Component));
    
    component->type = type;
    component->vtable = vtable;
    component->gameObject = gameObject;
    component->enabled = true;
    component->id = 0; // Will be set by registry
    
    return COMPONENT_OK;
}

void component_destroy(Component* component) {
    if (component) {
        memset(component, 0, sizeof(Component));
    }
}

void component_set_enabled(Component* component, bool enabled) {
    if (!component) return;
    
    bool wasEnabled = component->enabled;
    component->enabled = enabled;
    
    // Call lifecycle events
    if (enabled && !wasEnabled) {
        component_call_on_enabled(component);
    } else if (!enabled && wasEnabled) {
        component_call_on_disabled(component);
    }
}

bool component_is_enabled(const Component* component) {
    return component ? component->enabled : false;
}

bool component_is_type(const Component* component, ComponentType type) {
    return component ? (component->type & type) != 0 : false;
}

const char* component_type_to_string(ComponentType type) {
    switch (type) {
        case COMPONENT_TYPE_TRANSFORM: return "Transform";
        case COMPONENT_TYPE_SPRITE: return "Sprite";
        case COMPONENT_TYPE_COLLISION: return "Collision";
        case COMPONENT_TYPE_SCRIPT: return "Script";
        case COMPONENT_TYPE_AUDIO: return "Audio";
        case COMPONENT_TYPE_ANIMATION: return "Animation";
        case COMPONENT_TYPE_PARTICLES: return "Particles";
        case COMPONENT_TYPE_UI: return "UI";
        default: return "Unknown";
    }
}

// Safe virtual function calls
void component_call_update(Component* component, float deltaTime) {
    if (component && component->enabled && component->vtable && component->vtable->update) {
        component->vtable->update(component, deltaTime);
    }
}

void component_call_render(Component* component) {
    if (component && component->enabled && component->vtable && component->vtable->render) {
        component->vtable->render(component);
    }
}

void component_call_on_enabled(Component* component) {
    if (component && component->vtable && component->vtable->onEnabled) {
        component->vtable->onEnabled(component);
    }
}

void component_call_on_disabled(Component* component) {
    if (component && component->vtable && component->vtable->onDisabled) {
        component->vtable->onDisabled(component);
    }
}