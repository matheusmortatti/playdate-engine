#ifndef SPATIAL_GRID_H
#define SPATIAL_GRID_H

#include "../core/game_object.h"
#include "../core/memory_pool.h"
#include <stdint.h>
#include <stdbool.h>

// Grid configuration
#define DEFAULT_CELL_SIZE 64        // 64x64 pixel cells
#define MAX_OBJECTS_PER_CELL 32     // Maximum objects per cell
#define MAX_GRID_WIDTH 256          // Maximum grid width in cells
#define MAX_GRID_HEIGHT 256         // Maximum grid height in cells

// Forward declarations
typedef struct SpatialGrid SpatialGrid;
typedef struct GridCell GridCell;
typedef struct SpatialQuery SpatialQuery;

// Object entry in grid cell
typedef struct GridObjectEntry {
    GameObject* gameObject;
    struct GridObjectEntry* next;
    uint32_t cellX, cellY;          // Current cell coordinates
    bool staticObject;              // Optimization hint
} GridObjectEntry;

// Grid cell structure
typedef struct GridCell {
    GridObjectEntry* objects;       // Linked list of objects in cell
    uint32_t objectCount;          // Number of objects in cell
    uint32_t maxObjects;           // Maximum before performance warning
    bool dirty;                    // Needs spatial optimization
} GridCell;

// Spatial grid structure
typedef struct SpatialGrid {
    GridCell* cells;               // 2D array of cells (width * height)
    uint32_t cellSize;             // Size of each cell in world units
    uint32_t gridWidth;            // Number of cells horizontally
    uint32_t gridHeight;           // Number of cells vertically
    float worldWidth;              // Total world width
    float worldHeight;             // Total world height
    float offsetX, offsetY;        // World origin offset
    
    // Object management
    ObjectPool entryPool;          // Pool for GridObjectEntry allocations
    GridObjectEntry** objectLookup; // Fast object-to-entry lookup
    uint32_t totalObjects;         // Total objects in grid
    uint32_t maxObjects;           // Maximum objects supported
    
    // Performance statistics
    uint32_t queriesPerFrame;
    uint32_t collisionChecksPerFrame;
    float lastUpdateTime;
    uint32_t cellsWithObjects;
    
    // Configuration
    bool enableStaticOptimization;
    bool enableFrustumCulling;
    uint32_t maxObjectsPerCell;
    
} SpatialGrid;

// Query result structure
typedef struct SpatialQuery {
    GameObject** results;          // Array of found objects
    uint32_t resultCount;          // Number of results found
    uint32_t maxResults;           // Maximum results capacity
    float queryX, queryY;          // Query center
    float queryRadius;             // Query radius (for circle queries)
    float queryWidth, queryHeight; // Query size (for rectangle queries)
    bool includeStatic;            // Include static objects in results
} SpatialQuery;

// Grid management
SpatialGrid* spatial_grid_create(uint32_t cellSize, uint32_t gridWidth, uint32_t gridHeight,
                               float worldOffsetX, float worldOffsetY, uint32_t maxObjects);
void spatial_grid_destroy(SpatialGrid* grid);

// Object management
bool spatial_grid_add_object(SpatialGrid* grid, GameObject* gameObject);
bool spatial_grid_remove_object(SpatialGrid* grid, GameObject* gameObject);
bool spatial_grid_update_object(SpatialGrid* grid, GameObject* gameObject);
void spatial_grid_mark_static(SpatialGrid* grid, GameObject* gameObject, bool isStatic);

// Spatial queries
SpatialQuery* spatial_query_create(uint32_t maxResults);
void spatial_query_destroy(SpatialQuery* query);

uint32_t spatial_grid_query_circle(SpatialGrid* grid, float centerX, float centerY, 
                                  float radius, SpatialQuery* query);
uint32_t spatial_grid_query_rectangle(SpatialGrid* grid, float x, float y, 
                                     float width, float height, SpatialQuery* query);
uint32_t spatial_grid_query_line(SpatialGrid* grid, float x1, float y1, 
                                float x2, float y2, SpatialQuery* query);

// Grid utilities
void spatial_grid_get_cell_bounds(SpatialGrid* grid, uint32_t cellX, uint32_t cellY,
                                float* minX, float* minY, float* maxX, float* maxY);
bool spatial_grid_world_to_cell(SpatialGrid* grid, float worldX, float worldY,
                              uint32_t* cellX, uint32_t* cellY);
void spatial_grid_cell_to_world(SpatialGrid* grid, uint32_t cellX, uint32_t cellY,
                              float* worldX, float* worldY);

// Performance and debugging
void spatial_grid_print_stats(SpatialGrid* grid);
void spatial_grid_reset_frame_stats(SpatialGrid* grid);
uint32_t spatial_grid_get_memory_usage(SpatialGrid* grid);

// Fast inline helpers
static inline GridCell* spatial_grid_get_cell(SpatialGrid* grid, uint32_t cellX, uint32_t cellY) {
    if (cellX >= grid->gridWidth || cellY >= grid->gridHeight) {
        return NULL;
    }
    return &grid->cells[cellY * grid->gridWidth + cellX];
}

static inline bool spatial_grid_is_valid_cell(SpatialGrid* grid, uint32_t cellX, uint32_t cellY) {
    return cellX < grid->gridWidth && cellY < grid->gridHeight;
}

#endif // SPATIAL_GRID_H