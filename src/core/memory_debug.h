#ifndef MEMORY_DEBUG_H
#define MEMORY_DEBUG_H

#include "memory_pool.h"

// Debug configuration
#ifdef DEBUG
    #define ENABLE_MEMORY_TRACKING 1
    #define ENABLE_MEMORY_STATS 1
#else
    #define ENABLE_MEMORY_TRACKING 0
    #define ENABLE_MEMORY_STATS 0
#endif

#define MAX_TRACKED_POOLS 32

typedef struct MemoryStats {
    uint32_t totalPools;
    uint32_t totalAllocatedObjects;
    uint32_t totalMemoryUsed;
    uint32_t peakMemoryUsed;
    uint32_t totalAllocations;
    uint32_t totalDeallocations;
} MemoryStats;

typedef struct PoolRegistry {
    ObjectPool* pools[MAX_TRACKED_POOLS];
    uint32_t poolCount;
    MemoryStats globalStats;
    MemoryStats snapshot;
    bool hasSnapshot;
} PoolRegistry;

// Global memory tracking
void memory_debug_init(void);
void memory_debug_shutdown(void);
void memory_debug_register_pool(ObjectPool* pool);
void memory_debug_unregister_pool(ObjectPool* pool);

// Statistics and reporting
MemoryStats memory_debug_get_stats(void);
void memory_debug_print_report(void);
void memory_debug_print_pool_stats(const ObjectPool* pool);

// Leak detection
void memory_debug_snapshot(void);
void memory_debug_compare_snapshots(void);

// Internal helpers
void memory_debug_update_stats(void);

#endif // MEMORY_DEBUG_H