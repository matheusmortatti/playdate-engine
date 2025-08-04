#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ARM Cortex-M7 optimal alignment
#define MEMORY_ALIGNMENT 16
#define ALIGN_SIZE(size) (((size) + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1))

typedef enum {
    POOL_OK = 0,
    POOL_ERROR_NULL_POINTER,
    POOL_ERROR_OUT_OF_MEMORY,
    POOL_ERROR_INVALID_SIZE,
    POOL_ERROR_POOL_FULL,
    POOL_ERROR_INVALID_INDEX,
    POOL_ERROR_DOUBLE_FREE
} PoolResult;

typedef struct ObjectPool {
    void* memory;              // Pre-allocated aligned memory block
    uint32_t* freeList;        // Stack of free indices
    uint8_t* objectStates;     // Per-object allocation state (debug)
    uint32_t elementSize;      // Size per object (aligned)
    uint32_t capacity;         // Maximum number of objects
    uint32_t freeCount;        // Number of free objects
    uint32_t freeHead;         // Index of next free object
    const char* debugName;     // Pool identifier for debugging
    
    // Statistics for profiling
    uint32_t totalAllocations;
    uint32_t totalDeallocations;
    uint32_t peakUsage;
} ObjectPool;

// Core pool operations
PoolResult object_pool_init(ObjectPool* pool, uint32_t elementSize, 
                           uint32_t capacity, const char* debugName);
void object_pool_destroy(ObjectPool* pool);

void* object_pool_alloc(ObjectPool* pool);
PoolResult object_pool_free(ObjectPool* pool, void* object);

// Pool queries
uint32_t object_pool_get_used_count(const ObjectPool* pool);
uint32_t object_pool_get_free_count(const ObjectPool* pool);
float object_pool_get_usage_percent(const ObjectPool* pool);

// Object validation
bool object_pool_owns_object(const ObjectPool* pool, const void* object);
uint32_t object_pool_get_object_index(const ObjectPool* pool, const void* object);

#endif // MEMORY_POOL_H