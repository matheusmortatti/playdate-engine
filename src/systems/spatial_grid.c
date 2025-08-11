#include "spatial_grid.h"
#include "../components/transform_component.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Grid management
SpatialGrid* spatial_grid_create(uint32_t cellSize, uint32_t gridWidth, uint32_t gridHeight,
                               float worldOffsetX, float worldOffsetY, uint32_t maxObjects) {
    if (cellSize == 0 || gridWidth == 0 || gridHeight == 0 || maxObjects == 0) {
        return NULL;
    }
    
    SpatialGrid* grid = malloc(sizeof(SpatialGrid));
    if (!grid) {
        return NULL;
    }
    
    memset(grid, 0, sizeof(SpatialGrid));
    
    // Initialize grid properties
    grid->cellSize = cellSize;
    grid->gridWidth = gridWidth;
    grid->gridHeight = gridHeight;
    grid->worldWidth = gridWidth * cellSize;
    grid->worldHeight = gridHeight * cellSize;
    grid->offsetX = worldOffsetX;
    grid->offsetY = worldOffsetY;
    grid->maxObjects = maxObjects;
    grid->maxObjectsPerCell = MAX_OBJECTS_PER_CELL;
    grid->enableStaticOptimization = true;
    grid->enableFrustumCulling = true;
    
    // Allocate cells
    uint32_t totalCells = gridWidth * gridHeight;
    grid->cells = calloc(totalCells, sizeof(GridCell));
    if (!grid->cells) {
        free(grid);
        return NULL;
    }
    
    // Initialize each cell
    for (uint32_t i = 0; i < totalCells; i++) {
        grid->cells[i].maxObjects = MAX_OBJECTS_PER_CELL;
    }
    
    // Initialize object entry pool
    PoolResult poolResult = object_pool_init(&grid->entryPool, 
                                           sizeof(GridObjectEntry),
                                           maxObjects,
                                           "SpatialGridEntries");
    if (poolResult != POOL_OK) {
        free(grid->cells);
        free(grid);
        return NULL;
    }
    
    // Allocate object lookup table
    grid->objectLookup = calloc(maxObjects, sizeof(GridObjectEntry*));
    if (!grid->objectLookup) {
        object_pool_destroy(&grid->entryPool);
        free(grid->cells);
        free(grid);
        return NULL;
    }
    
    return grid;
}

void spatial_grid_destroy(SpatialGrid* grid) {
    if (!grid) return;
    
    // Clean up all cells
    uint32_t totalCells = grid->gridWidth * grid->gridHeight;
    for (uint32_t i = 0; i < totalCells; i++) {
        GridCell* cell = &grid->cells[i];
        GridObjectEntry* entry = cell->objects;
        while (entry) {
            GridObjectEntry* next = entry->next;
            object_pool_free(&grid->entryPool, entry);
            entry = next;
        }
    }
    
    object_pool_destroy(&grid->entryPool);
    free(grid->objectLookup);
    free(grid->cells);
    free(grid);
}

// Object management
bool spatial_grid_add_object(SpatialGrid* grid, GameObject* gameObject) {
    if (!grid || !gameObject || !gameObject->transform) {
        return false;
    }
    
    // Get object position
    float x, y;
    transform_component_get_position(gameObject->transform, &x, &y);
    
    // Convert to cell coordinates
    uint32_t cellX, cellY;
    if (!spatial_grid_world_to_cell(grid, x, y, &cellX, &cellY)) {
        return false; // Object outside grid bounds
    }
    
    // Allocate grid entry
    GridObjectEntry* entry = (GridObjectEntry*)object_pool_alloc(&grid->entryPool);
    if (!entry) {
        return false; // Pool full
    }
    
    // Initialize entry
    entry->gameObject = gameObject;
    entry->cellX = cellX;
    entry->cellY = cellY;
    entry->staticObject = game_object_is_static(gameObject);
    entry->next = NULL;
    
    // Add to cell
    GridCell* cell = spatial_grid_get_cell(grid, cellX, cellY);
    if (!cell) {
        object_pool_free(&grid->entryPool, entry);
        return false;
    }
    
    // Insert at head of linked list
    entry->next = cell->objects;
    cell->objects = entry;
    cell->objectCount++;
    
    // Update grid statistics
    grid->totalObjects++;
    if (cell->objectCount == 1) {
        grid->cellsWithObjects++;
    }
    
    // Store in lookup table for fast removal
    uint32_t objectId = game_object_get_id(gameObject);
    if (objectId < grid->maxObjects) {
        grid->objectLookup[objectId] = entry;
    }
    
    // Mark cell as dirty if it has too many objects
    if (cell->objectCount > cell->maxObjects) {
        cell->dirty = true;
    }
    
    return true;
}

bool spatial_grid_remove_object(SpatialGrid* grid, GameObject* gameObject) {
    if (!grid || !gameObject) {
        return false;
    }
    
    uint32_t objectId = game_object_get_id(gameObject);
    if (objectId >= grid->maxObjects) {
        return false;
    }
    
    GridObjectEntry* entry = grid->objectLookup[objectId];
    if (!entry) {
        return false; // Object not in grid
    }
    
    // Get cell
    GridCell* cell = spatial_grid_get_cell(grid, entry->cellX, entry->cellY);
    if (!cell) {
        return false;
    }
    
    // Remove from linked list
    if (cell->objects == entry) {
        cell->objects = entry->next;
    } else {
        GridObjectEntry* prev = cell->objects;
        while (prev && prev->next != entry) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = entry->next;
        }
    }
    
    // Update statistics
    cell->objectCount--;
    grid->totalObjects--;
    if (cell->objectCount == 0) {
        grid->cellsWithObjects--;
        cell->dirty = false;
    }
    
    // Clear lookup
    grid->objectLookup[objectId] = NULL;
    
    // Return entry to pool
    object_pool_free(&grid->entryPool, entry);
    
    return true;
}

bool spatial_grid_update_object(SpatialGrid* grid, GameObject* gameObject) {
    if (!grid || !gameObject || !gameObject->transform) {
        return false;
    }
    
    // Skip static objects unless explicitly marked for update
    if (game_object_is_static(gameObject)) {
        return true;
    }
    
    uint32_t objectId = game_object_get_id(gameObject);
    if (objectId >= grid->maxObjects) {
        return false;
    }
    
    GridObjectEntry* entry = grid->objectLookup[objectId];
    if (!entry) {
        // Object not in grid, try to add it
        return spatial_grid_add_object(grid, gameObject);
    }
    
    // Get new position
    float x, y;
    transform_component_get_position(gameObject->transform, &x, &y);
    
    // Convert to cell coordinates
    uint32_t newCellX, newCellY;
    if (!spatial_grid_world_to_cell(grid, x, y, &newCellX, &newCellY)) {
        // Object moved outside grid, remove it
        return spatial_grid_remove_object(grid, gameObject);
    }
    
    // Check if object moved to a different cell
    if (newCellX == entry->cellX && newCellY == entry->cellY) {
        return true; // No movement, nothing to do
    }
    
    // Remove from old cell and add to new cell
    spatial_grid_remove_object(grid, gameObject);
    return spatial_grid_add_object(grid, gameObject);
}

void spatial_grid_mark_static(SpatialGrid* grid, GameObject* gameObject, bool isStatic) {
    (void)grid;
    (void)gameObject;
    (void)isStatic;
}

// Spatial queries
SpatialQuery* spatial_query_create(uint32_t maxResults) {
    if (maxResults == 0) {
        return NULL;
    }
    
    SpatialQuery* query = malloc(sizeof(SpatialQuery));
    if (!query) {
        return NULL;
    }
    
    memset(query, 0, sizeof(SpatialQuery));
    
    // Allocate results array
    query->results = malloc(maxResults * sizeof(GameObject*));
    if (!query->results) {
        free(query);
        return NULL;
    }
    
    query->maxResults = maxResults;
    query->resultCount = 0;
    query->includeStatic = true;
    
    return query;
}

void spatial_query_destroy(SpatialQuery* query) {
    if (!query) return;
    
    free(query->results);
    free(query);
}

uint32_t spatial_grid_query_circle(SpatialGrid* grid, float centerX, float centerY, 
                                  float radius, SpatialQuery* query) {
    if (!grid || !query || radius <= 0) {
        return 0;
    }
    
    query->resultCount = 0;
    query->queryX = centerX;
    query->queryY = centerY;
    query->queryRadius = radius;
    
    // Calculate affected cells
    uint32_t minCellX, minCellY, maxCellX, maxCellY;
    float minX = centerX - radius;
    float minY = centerY - radius;
    float maxX = centerX + radius;
    float maxY = centerY + radius;
    
    if (!spatial_grid_world_to_cell(grid, minX, minY, &minCellX, &minCellY) ||
        !spatial_grid_world_to_cell(grid, maxX, maxY, &maxCellX, &maxCellY)) {
        return 0; // Query outside grid bounds
    }
    
    float radiusSquared = radius * radius;
    
    // Iterate through affected cells
    for (uint32_t cellY = minCellY; cellY <= maxCellY; cellY++) {
        for (uint32_t cellX = minCellX; cellX <= maxCellX; cellX++) {
            GridCell* cell = spatial_grid_get_cell(grid, cellX, cellY);
            if (!cell || cell->objectCount == 0) continue;
            
            // Check each object in cell
            GridObjectEntry* entry = cell->objects;
            while (entry && query->resultCount < query->maxResults) {
                GameObject* obj = entry->gameObject;
                
                // Skip inactive objects
                if (!game_object_is_active(obj)) {
                    entry = entry->next;
                    continue;
                }
                
                // Skip static objects if not requested
                if (!query->includeStatic && entry->staticObject) {
                    entry = entry->next;
                    continue;
                }
                
                // Get object position
                float objX, objY;
                transform_component_get_position(obj->transform, &objX, &objY);
                
                // Distance check
                float dx = objX - centerX;
                float dy = objY - centerY;
                float distanceSquared = dx * dx + dy * dy;
                
                if (distanceSquared <= radiusSquared) {
                    query->results[query->resultCount] = obj;
                    query->resultCount++;
                }
                
                entry = entry->next;
            }
        }
    }
    
    grid->queriesPerFrame++;
    return query->resultCount;
}

uint32_t spatial_grid_query_rectangle(SpatialGrid* grid, float x, float y, 
                                     float width, float height, SpatialQuery* query) {
    (void)grid;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)query;
    return 0;
}

uint32_t spatial_grid_query_line(SpatialGrid* grid, float x1, float y1, 
                                float x2, float y2, SpatialQuery* query) {
    (void)grid;
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    (void)query;
    return 0;
}

// Grid utilities
void spatial_grid_get_cell_bounds(SpatialGrid* grid, uint32_t cellX, uint32_t cellY,
                                float* minX, float* minY, float* maxX, float* maxY) {
    (void)grid;
    (void)cellX;
    (void)cellY;
    (void)minX;
    (void)minY;
    (void)maxX;
    (void)maxY;
}

bool spatial_grid_world_to_cell(SpatialGrid* grid, float worldX, float worldY,
                              uint32_t* cellX, uint32_t* cellY) {
    if (!grid || !cellX || !cellY) {
        return false;
    }
    
    // Adjust for grid offset
    float adjustedX = worldX - grid->offsetX;
    float adjustedY = worldY - grid->offsetY;
    
    // Check bounds
    if (adjustedX < 0 || adjustedY < 0 || 
        adjustedX >= grid->worldWidth || adjustedY >= grid->worldHeight) {
        return false;
    }
    
    *cellX = (uint32_t)(adjustedX / grid->cellSize);
    *cellY = (uint32_t)(adjustedY / grid->cellSize);
    
    return true;
}

void spatial_grid_cell_to_world(SpatialGrid* grid, uint32_t cellX, uint32_t cellY,
                              float* worldX, float* worldY) {
    (void)grid;
    (void)cellX;
    (void)cellY;
    (void)worldX;
    (void)worldY;
}

// Performance and debugging
void spatial_grid_print_stats(SpatialGrid* grid) {
    (void)grid;
}

void spatial_grid_reset_frame_stats(SpatialGrid* grid) {
    (void)grid;
}

uint32_t spatial_grid_get_memory_usage(SpatialGrid* grid) {
    (void)grid;
    return 0;
}