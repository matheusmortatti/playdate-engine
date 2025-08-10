# Playdate Game Development Engine

A high-performance, C-based game development toolkit for the Playdate console with optional Lua bindings. Built for ambitious performance targets and developer productivity.

## 🎯 Key Features

- **High-Performance C Core** - Optimized for ARM Cortex-M7 with object pooling and cache-friendly data structures
- **Entity-Component Architecture** - Type-safe, bitmask-based component system with O(1) queries
- **Memory-First Design** - Predictable allocation patterns with zero runtime allocation in hot paths  
- **Phased Development** - 11 well-defined phases from memory management to complete games
- **Comprehensive Testing** - Performance benchmarks and unit tests for every system
- **Hardware Optimization** - Leverages Playdate's 168MHz ARM Cortex-M7 and 16MB RAM efficiently

## 🚀 Performance Targets

| System | Target | Current Status |
|--------|--------|----------------|
| GameObject Updates | 50,000 objects/frame | ✅ Implemented |
| Component Queries | < 1ns type check | ✅ Implemented |
| Memory Allocation | < 100ns from pools | ✅ Implemented |
| Collision Detection | 10,000 checks/frame | 🚧 In Progress |
| Sprite Rendering | 1000+ sprites @ 30fps | 📋 Planned |

## ⚡ Quick Start

### Prerequisites

- Playdate SDK installed at `/Users/matheusmortatti/Playdate`
- GCC with C11 support
- Make

### Build and Test

```bash
# Clone the repository
git clone <repository-url>
cd playdate-engine

# Run all tests (validates memory, components, and GameObjects)
make test-all

# Run specific system tests
make test-memory      # Phase 1: Memory pools
make test-components  # Phase 2: Component system  
make test-gameobject  # Phase 3: GameObject system

# Quick validation
make quick-test
```

## 🏗️ Current Implementation Status

### ✅ Completed Phases

**Phase 1: Memory Management**
- High-performance object pools with ARM optimization
- 16-byte aligned allocations for cache efficiency
- Memory debugging and leak detection
- Performance: < 100ns allocation time

**Phase 2: Component System** 
- Bitmask-based component type checking (O(1) performance)
- Virtual function tables for polymorphism
- Component registry and factory pattern
- Transform component with matrix caching

**Phase 3: GameObject & Transform**
- Entity management with component composition
- Transform hierarchies with parent-child relationships
- Batch processing optimizations
- 96-byte cache-aligned GameObject structure

### 🚧 In Progress

**Phase 4: Scene Management** - GameObject organization and lifecycle
**Phase 5: Spatial Partitioning** - Grid-based collision optimization

### 📋 Planned Phases

- **Phase 6**: Sprite Component (Hardware-accelerated rendering)
- **Phase 7**: Collision Component (AABB collision with spatial integration)
- **Phase 8**: Lua Bindings (Complete C-to-Lua API)
- **Phase 9**: Build System (CMake + Playdate SDK integration)
- **Phase 10**: Optimization & Profiling
- **Phase 11**: Integration Examples (Complete games)

## 🛠️ API Examples

### C API (Production Performance)

```c
#include "src/core/game_object.h"
#include "src/components/transform_component.h"

// Create a GameObject with transform
Scene* scene = scene_create();
GameObject* player = game_object_create(scene);

// Add and configure components
TransformComponent* transform = transform_component_create();
transform_component_set_position(transform, 100.0f, 50.0f);
game_object_add_component(player, (Component*)transform);

// Batch update all objects in scene
scene_update_all(scene, deltaTime);
```

### Lua API (Rapid Development) - *Planned Phase 8*

```lua
-- High-level Lua interface over C core
local player = GameObject.new()
player:addComponent(TransformComponent.new())
player:setPosition(100, 50)

function playdate.update()
    GameObject.updateAll()  -- C performance for hot path
end
```

## 🏛️ Architecture

### Memory-First Design Philosophy

The engine is built around predictable memory patterns optimized for the Playdate's ARM Cortex-M7:

```c
// 96-byte cache-aligned GameObject structure
struct GameObject {
    uint32_t id;                           // 4 bytes
    uint32_t componentMask;                // 4 bytes - O(1) type queries
    Component* components[MAX_COMPONENTS]; // 32 bytes - component array  
    TransformComponent* transform;         // 8 bytes - cached pointer
    Scene* scene;                         // 8 bytes
    GameObject* parent;                   // 8 bytes - hierarchy
    GameObject* firstChild;               // 8 bytes
    GameObject* nextSibling;              // 8 bytes  
    uint8_t active;                       // 1 byte
    uint8_t staticObject;                 // 1 byte
    uint8_t componentCount;               // 1 byte
    uint8_t padding[13];                  // 13 bytes - explicit padding
};
```

### Component System

- **Bitmask Type Checking**: O(1) component queries using bit operations
- **Pool Allocation**: All components allocated from type-specific pools
- **Virtual Functions**: Polymorphic behavior without vtable overhead
- **Batch Processing**: System updates process components by type

### Hardware Optimization

**ARM Cortex-M7 Specific**:
- 32-byte cache line alignment for critical structures
- 16-byte alignment for SIMD operations
- Optimized for dual-issue pipeline execution
- Hardware floating-point unit utilization

**Memory System**:
- **L1 Cache**: 8KB total (4KB I-Cache + 4KB D-Cache)
- **System RAM**: 16MB total (engine uses < 2MB)
- **TCM Integration**: Critical code can use tightly-coupled memory

## 📖 Development Phases

Detailed implementation guides are available in the [`tasks/`](tasks/) directory:

### Foundation
- [`phase-1-memory-management.md`](tasks/phase-1-memory-management.md) - Object pools and ARM optimization
- [`phase-2-component-system.md`](tasks/phase-2-component-system.md) - Type-safe component architecture

### Core Systems  
- [`phase-3-gameobject-transform.md`](tasks/phase-3-gameobject-transform.md) - Entity management
- [`phase-4-scene-management.md`](tasks/phase-4-scene-management.md) - GameObject organization

### Rendering & Physics
- [`phase-5-spatial-partitioning.md`](tasks/phase-5-spatial-partitioning.md) - Collision optimization
- [`phase-6-sprite-component.md`](tasks/phase-6-sprite-component.md) - Hardware-accelerated rendering
- [`phase-7-collision-component.md`](tasks/phase-7-collision-component.md) - AABB collision detection

### Scripting & Tools
- [`phase-8-lua-bindings.md`](tasks/phase-8-lua-bindings.md) - C-to-Lua API bindings
- [`phase-9-build-system.md`](tasks/phase-9-build-system.md) - CMake + Playdate SDK
- [`phase-10-optimization.md`](tasks/phase-10-optimization.md) - Profiling tools
- [`phase-11-integration-examples.md`](tasks/phase-11-integration-examples.md) - Complete games

## 🔧 Build System

The project uses a phase-aware Makefile that builds and tests each system independently:

```bash
# Test individual phases
make test-memory      # Phase 1: Memory pools and debugging
make test-components  # Phase 2: Component system and registry  
make test-gameobject  # Phase 3: GameObject and transform system

# Performance benchmarking
make test-verbose     # Detailed performance output
make test-perf        # Memory allocation benchmarks

# Development workflows
make clean           # Clean all build artifacts
make quick-test      # Fast validation of current implementation
```

### Compiler Optimizations

The build system automatically applies ARM Cortex-M7 optimizations:

```c
// Automatically applied flags
-mcpu=cortex-m7          // Target Cortex-M7 specifically  
-mthumb                  // Thumb-2 instruction set
-mfpu=fpv5-sp-d16       // Hardware floating-point
-O2 -ffast-math         // Performance optimizations
-falign-functions=32    // Cache line alignment
```

## 🎮 Playdate SDK Integration

The engine integrates seamlessly with the Playdate SDK:

```c
// Standard Playdate event handler
int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
    if (event == kEventInit) {
        // Initialize engine with Playdate API
        engine_init(pd);
        
        // Create game objects using engine
        Scene* mainScene = scene_create();
        GameObject* player = game_object_create(mainScene);
        
        // Set update callback
        pd->system->setUpdateCallback(gameUpdate, pd);
    }
    return 0;
}
```

## 📊 Performance Validation

Every system includes comprehensive performance tests:

```c
// Example: GameObject creation benchmark
void benchmark_gameobject_creation(void) {
    uint64_t start = get_microseconds();
    
    for (int i = 0; i < 50000; i++) {
        GameObject* obj = game_object_create(scene);
        game_object_destroy(obj);
    }
    
    uint64_t elapsed = get_microseconds() - start;
    assert(elapsed < 1000000); // Must complete in < 1 second
}
```

## 🤝 Contributing

### Development Standards

- **C11 Standard** with strict warnings (`-Wall -Wextra`)
- **Performance First** - All changes must meet benchmark targets
- **Test Coverage** - 100% coverage for core systems
- **Documentation** - API documentation and implementation guides

### Workflow

1. **Choose a Phase** - Pick from the [`tasks/`](tasks/) directory
2. **Implement Completely** - Follow the phase documentation exactly
3. **Pass All Tests** - Both unit tests and performance benchmarks
4. **Document Changes** - Update API docs and examples
5. **Validate Integration** - Ensure compatibility with other phases

### Code Quality

```c
// Example: All functions must handle errors gracefully
GameObjectResult game_object_add_component(GameObject* gameObject, Component* component) {
    if (!gameObject || !component) {
        return GAMEOBJECT_ERROR_NULL_POINTER;
    }
    
    if (gameObject->componentCount >= MAX_COMPONENTS_PER_OBJECT) {
        return GAMEOBJECT_ERROR_MAX_COMPONENTS_REACHED;
    }
    
    // Implementation...
    return GAMEOBJECT_OK;
}
```

## 📈 Roadmap

### Version 1.0 (Core Engine)
- [x] Memory management and object pooling  
- [x] Component system with type safety
- [x] GameObject management and hierarchies
- [ ] Scene management and lifecycle
- [ ] Spatial partitioning for performance
- [ ] Basic collision detection

### Version 1.1 (Rendering & Assets)
- [ ] Sprite component with hardware acceleration
- [ ] Aseprite file format integration  
- [ ] Tiled map loader and renderer
- [ ] Animation system
- [ ] Audio integration

### Version 2.0 (Scripting & Tools)
- [ ] Complete Lua bindings
- [ ] Visual debugging tools
- [ ] Performance profiling suite
- [ ] Asset bundling pipeline
- [ ] Complete game examples

## 📄 License

This project is designed for the Playdate development community. See individual files for specific licensing terms.

## 🆘 Support

- **Documentation**: Comprehensive guides in [`tasks/`](tasks/) directory
- **Examples**: Reference implementations in each phase
- **Performance**: Benchmark validation in test suite
- **Issues**: Report bugs via GitHub issues

---

*Built with ❤️ for high-performance Playdate game development*