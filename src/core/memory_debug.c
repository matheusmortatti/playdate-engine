#include "memory_debug.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static PoolRegistry g_registry = {0};

void memory_debug_init(void) {
    memset(&g_registry, 0, sizeof(PoolRegistry));
    g_registry.poolCount = 0;
    g_registry.hasSnapshot = false;
}

void memory_debug_shutdown(void) {
    // Print final report if there are active pools
    if (g_registry.poolCount > 0) {
        printf("=== Memory Debug Shutdown Report ===\n");
        memory_debug_print_report();
        
        // Check for potential leaks
        if (g_registry.globalStats.totalAllocatedObjects > 0) {
            printf("WARNING: %u objects still allocated at shutdown\n", 
                   g_registry.globalStats.totalAllocatedObjects);
        }
    }
    
    memset(&g_registry, 0, sizeof(PoolRegistry));
}

void memory_debug_register_pool(ObjectPool* pool) {
    if (!pool) return;
    
#if ENABLE_MEMORY_TRACKING
    if (g_registry.poolCount >= MAX_TRACKED_POOLS) {
        printf("WARNING: Cannot register pool '%s' - registry full\n", 
               pool->debugName ? pool->debugName : "unnamed");
        return;
    }
    
    g_registry.pools[g_registry.poolCount] = pool;
    g_registry.poolCount++;
    
    printf("Registered memory pool: %s (capacity: %u, element size: %u bytes)\n",
           pool->debugName ? pool->debugName : "unnamed",
           pool->capacity, pool->elementSize);
#endif
}

void memory_debug_unregister_pool(ObjectPool* pool) {
    if (!pool) return;
    
#if ENABLE_MEMORY_TRACKING
    for (uint32_t i = 0; i < g_registry.poolCount; i++) {
        if (g_registry.pools[i] == pool) {
            // Check for leaks before unregistering
            uint32_t usedCount = object_pool_get_used_count(pool);
            if (usedCount > 0) {
                printf("WARNING: Pool '%s' has %u objects still allocated\n",
                       pool->debugName ? pool->debugName : "unnamed", usedCount);
            }
            
            // Shift remaining pools down
            for (uint32_t j = i; j < g_registry.poolCount - 1; j++) {
                g_registry.pools[j] = g_registry.pools[j + 1];
            }
            g_registry.poolCount--;
            
            printf("Unregistered memory pool: %s\n",
                   pool->debugName ? pool->debugName : "unnamed");
            return;
        }
    }
    
    printf("WARNING: Attempted to unregister unknown pool\n");
#endif
}

void memory_debug_update_stats(void) {
#if ENABLE_MEMORY_STATS
    memset(&g_registry.globalStats, 0, sizeof(MemoryStats));
    g_registry.globalStats.totalPools = g_registry.poolCount;
    
    for (uint32_t i = 0; i < g_registry.poolCount; i++) {
        ObjectPool* pool = g_registry.pools[i];
        if (!pool) continue;
        
        uint32_t usedCount = object_pool_get_used_count(pool);
        uint32_t memoryUsed = usedCount * pool->elementSize;
        
        g_registry.globalStats.totalAllocatedObjects += usedCount;
        g_registry.globalStats.totalMemoryUsed += memoryUsed;
        g_registry.globalStats.totalAllocations += pool->totalAllocations;
        g_registry.globalStats.totalDeallocations += pool->totalDeallocations;
        
        // Track peak memory usage
        uint32_t peakMemory = pool->peakUsage * pool->elementSize;
        if (peakMemory > g_registry.globalStats.peakMemoryUsed) {
            g_registry.globalStats.peakMemoryUsed = peakMemory;
        }
    }
#endif
}

MemoryStats memory_debug_get_stats(void) {
    memory_debug_update_stats();
    return g_registry.globalStats;
}

void memory_debug_print_report(void) {
#if ENABLE_MEMORY_STATS
    memory_debug_update_stats();
    
    printf("\n=== Memory Debug Report ===\n");
    printf("Total Pools: %u\n", g_registry.globalStats.totalPools);
    printf("Total Allocated Objects: %u\n", g_registry.globalStats.totalAllocatedObjects);
    printf("Total Memory Used: %u bytes (%.2f KB)\n", 
           g_registry.globalStats.totalMemoryUsed,
           g_registry.globalStats.totalMemoryUsed / 1024.0f);
    printf("Peak Memory Used: %u bytes (%.2f KB)\n",
           g_registry.globalStats.peakMemoryUsed,
           g_registry.globalStats.peakMemoryUsed / 1024.0f);
    printf("Total Allocations: %u\n", g_registry.globalStats.totalAllocations);
    printf("Total Deallocations: %u\n", g_registry.globalStats.totalDeallocations);
    
    if (g_registry.globalStats.totalAllocations != g_registry.globalStats.totalDeallocations) {
        printf("WARNING: Allocation/Deallocation mismatch detected!\n");
    }
    
    printf("\n--- Per-Pool Statistics ---\n");
    for (uint32_t i = 0; i < g_registry.poolCount; i++) {
        memory_debug_print_pool_stats(g_registry.pools[i]);
    }
    printf("=============================\n\n");
#else
    printf("Memory debug statistics disabled (release build)\n");
#endif
}

void memory_debug_print_pool_stats(const ObjectPool* pool) {
    if (!pool) return;
    
#if ENABLE_MEMORY_STATS
    uint32_t usedCount = object_pool_get_used_count(pool);
    float usagePercent = object_pool_get_usage_percent(pool);
    uint32_t memoryUsed = usedCount * pool->elementSize;
    uint32_t totalMemory = pool->capacity * pool->elementSize;
    
    printf("Pool: %s\n", pool->debugName ? pool->debugName : "unnamed");
    printf("  Capacity: %u objects\n", pool->capacity);
    printf("  Used: %u objects (%.1f%%)\n", usedCount, usagePercent);
    printf("  Element Size: %u bytes\n", pool->elementSize);
    printf("  Memory Used: %u / %u bytes (%.2f KB / %.2f KB)\n",
           memoryUsed, totalMemory,
           memoryUsed / 1024.0f, totalMemory / 1024.0f);
    printf("  Peak Usage: %u objects\n", pool->peakUsage);
    printf("  Total Allocations: %u\n", pool->totalAllocations);
    printf("  Total Deallocations: %u\n", pool->totalDeallocations);
    printf("\n");
#endif
}

void memory_debug_snapshot(void) {
#if ENABLE_MEMORY_TRACKING
    memory_debug_update_stats();
    g_registry.snapshot = g_registry.globalStats;
    g_registry.hasSnapshot = true;
    printf("Memory snapshot taken\n");
#endif
}

void memory_debug_compare_snapshots(void) {
#if ENABLE_MEMORY_TRACKING
    if (!g_registry.hasSnapshot) {
        printf("No snapshot available for comparison\n");
        return;
    }
    
    memory_debug_update_stats();
    MemoryStats current = g_registry.globalStats;
    MemoryStats snapshot = g_registry.snapshot;
    
    printf("\n=== Memory Snapshot Comparison ===\n");
    printf("Objects: %u -> %u (change: %+d)\n",
           snapshot.totalAllocatedObjects, current.totalAllocatedObjects,
           (int)(current.totalAllocatedObjects - snapshot.totalAllocatedObjects));
    printf("Memory: %u -> %u bytes (change: %+d bytes)\n",
           snapshot.totalMemoryUsed, current.totalMemoryUsed,
           (int)(current.totalMemoryUsed - snapshot.totalMemoryUsed));
    printf("Allocations: %u -> %u (change: %+d)\n",
           snapshot.totalAllocations, current.totalAllocations,
           (int)(current.totalAllocations - snapshot.totalAllocations));
    printf("Deallocations: %u -> %u (change: %+d)\n",
           snapshot.totalDeallocations, current.totalDeallocations,
           (int)(current.totalDeallocations - snapshot.totalDeallocations));
    
    // Check for potential leaks
    int objectDelta = (int)(current.totalAllocatedObjects - snapshot.totalAllocatedObjects);
    if (objectDelta > 0) {
        printf("WARNING: %d objects may have leaked since snapshot\n", objectDelta);
    } else if (objectDelta < 0) {
        printf("INFO: %d objects freed since snapshot\n", -objectDelta);
    } else {
        printf("INFO: No net change in allocated objects\n");
    }
    
    printf("=================================\n\n");
#endif
}