#include "memory_pool.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

PoolResult object_pool_init(ObjectPool* pool, uint32_t elementSize, 
                           uint32_t capacity, const char* debugName) {
    if (!pool || elementSize == 0 || capacity == 0) {
        return POOL_ERROR_NULL_POINTER;
    }
    
    // Align element size for optimal ARM performance
    uint32_t alignedSize = ALIGN_SIZE(elementSize);
    
    // Allocate aligned memory block
    pool->memory = aligned_alloc(MEMORY_ALIGNMENT, alignedSize * capacity);
    if (!pool->memory) {
        return POOL_ERROR_OUT_OF_MEMORY;
    }
    
    // Allocate free list (stack of indices)
    pool->freeList = malloc(capacity * sizeof(uint32_t));
    if (!pool->freeList) {
        free(pool->memory);
        return POOL_ERROR_OUT_OF_MEMORY;
    }
    
    // Debug state tracking
    pool->objectStates = calloc(capacity, sizeof(uint8_t));
    if (!pool->objectStates) {
        free(pool->memory);
        free(pool->freeList);
        return POOL_ERROR_OUT_OF_MEMORY;
    }
    
    // Initialize free list (all objects available)
    for (uint32_t i = 0; i < capacity; i++) {
        pool->freeList[i] = capacity - 1 - i; // Reverse order for cache efficiency
    }
    
    pool->elementSize = alignedSize;
    pool->capacity = capacity;
    pool->freeCount = capacity;
    pool->freeHead = 0;
    pool->debugName = debugName;
    
    // Reset statistics
    pool->totalAllocations = 0;
    pool->totalDeallocations = 0;
    pool->peakUsage = 0;
    
    return POOL_OK;
}

void object_pool_destroy(ObjectPool* pool) {
    if (pool) {
        free(pool->memory);
        free(pool->freeList);
        free(pool->objectStates);
        memset(pool, 0, sizeof(ObjectPool));
    }
}

void* object_pool_alloc(ObjectPool* pool) {
    if (!pool || pool->freeCount == 0) {
        return NULL;
    }
    
    // Pop from free list
    uint32_t index = pool->freeList[pool->freeHead];
    pool->freeHead++;
    pool->freeCount--;
    
    // Calculate object pointer
    void* object = (uint8_t*)pool->memory + (index * pool->elementSize);
    
    // Update debug state
    pool->objectStates[index] = 1; // Mark as allocated
    
    // Update statistics
    pool->totalAllocations++;
    uint32_t currentUsage = pool->capacity - pool->freeCount;
    if (currentUsage > pool->peakUsage) {
        pool->peakUsage = currentUsage;
    }
    
    return object;
}

PoolResult object_pool_free(ObjectPool* pool, void* object) {
    if (!pool || !object) {
        return POOL_ERROR_NULL_POINTER;
    }
    
    // Validate object belongs to this pool
    if (!object_pool_owns_object(pool, object)) {
        return POOL_ERROR_INVALID_INDEX;
    }
    
    uint32_t index = object_pool_get_object_index(pool, object);
    
    // Check for double-free
    if (pool->objectStates[index] == 0) {
        return POOL_ERROR_DOUBLE_FREE;
    }
    
    // Clear debug state
    pool->objectStates[index] = 0;
    
    // Push back to free list
    if (pool->freeHead == 0) {
        return POOL_ERROR_POOL_FULL; // This shouldn't happen
    }
    
    pool->freeHead--;
    pool->freeList[pool->freeHead] = index;
    pool->freeCount++;
    
    pool->totalDeallocations++;
    
    return POOL_OK;
}

uint32_t object_pool_get_used_count(const ObjectPool* pool) {
    if (!pool) return 0;
    return pool->capacity - pool->freeCount;
}

uint32_t object_pool_get_free_count(const ObjectPool* pool) {
    if (!pool) return 0;
    return pool->freeCount;
}

float object_pool_get_usage_percent(const ObjectPool* pool) {
    if (!pool || pool->capacity == 0) return 0.0f;
    return ((float)(pool->capacity - pool->freeCount) / (float)pool->capacity) * 100.0f;
}

bool object_pool_owns_object(const ObjectPool* pool, const void* object) {
    if (!pool || !object) return false;
    
    uintptr_t objectAddr = (uintptr_t)object;
    uintptr_t poolStart = (uintptr_t)pool->memory;
    uintptr_t poolEnd = poolStart + (pool->capacity * pool->elementSize);
    
    return (objectAddr >= poolStart && objectAddr < poolEnd && 
            ((objectAddr - poolStart) % pool->elementSize) == 0);
}

uint32_t object_pool_get_object_index(const ObjectPool* pool, const void* object) {
    if (!pool || !object) return UINT32_MAX;
    
    uintptr_t objectAddr = (uintptr_t)object;
    uintptr_t poolStart = (uintptr_t)pool->memory;
    
    return (uint32_t)((objectAddr - poolStart) / pool->elementSize);
}