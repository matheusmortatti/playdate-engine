#include "../../src/systems/spatial_grid.h"
#include "../../src/core/game_object.h"
#include "../../src/components/transform_component.h"
#include "../../src/core/component_registry.h"
#include "../../src/core/scene.h"
#include <assert.h>
#include <stdio.h>

void test_spatial_grid_creation(void) {
    SpatialGrid* grid = spatial_grid_create(64, 10, 10, 0, 0, 100);
    assert(grid != NULL);
    assert(grid->cellSize == 64);
    assert(grid->gridWidth == 10);
    assert(grid->gridHeight == 10);
    assert(grid->worldWidth == 640);
    assert(grid->worldHeight == 640);
    assert(grid->totalObjects == 0);
    
    spatial_grid_destroy(grid);
    printf("✓ Spatial grid creation test passed\n");
}

void test_object_addition_removal(void) {
    component_registry_init();
    transform_component_register();
    
    Scene* scene = scene_create("SpatialTest", 10);
    SpatialGrid* grid = spatial_grid_create(64, 10, 10, 0, 0, 100);
    
    // Create test object
    GameObject* obj = game_object_create(scene);
    game_object_set_position(obj, 100, 100); // Cell (1, 1)
    
    // Add to grid
    bool result = spatial_grid_add_object(grid, obj);
    assert(result == true);
    assert(grid->totalObjects == 1);
    
    // Remove from grid
    result = spatial_grid_remove_object(grid, obj);
    assert(result == true);
    assert(grid->totalObjects == 0);
    
    game_object_destroy(obj);
    spatial_grid_destroy(grid);
    scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Object addition/removal test passed\n");
}

void test_spatial_queries(void) {
    component_registry_init();
    transform_component_register();
    
    Scene* scene = scene_create("QueryTest", 10);
    SpatialGrid* grid = spatial_grid_create(64, 10, 10, 0, 0, 100);
    SpatialQuery* query = spatial_query_create(50);
    
    // Create test objects
    GameObject* obj1 = game_object_create(scene);
    GameObject* obj2 = game_object_create(scene);
    GameObject* obj3 = game_object_create(scene);
    
    game_object_set_position(obj1, 100, 100);
    game_object_set_position(obj2, 110, 110); // Close to obj1
    game_object_set_position(obj3, 300, 300); // Far from obj1
    
    spatial_grid_add_object(grid, obj1);
    spatial_grid_add_object(grid, obj2);
    spatial_grid_add_object(grid, obj3);
    
    // Query around obj1 position
    query->includeStatic = true;
    uint32_t found = spatial_grid_query_circle(grid, 100, 100, 50, query);
    
    assert(found >= 1); // Should find at least obj1
    assert(query->resultCount == found);
    
    // Verify obj1 is in results
    bool foundObj1 = false;
    for (uint32_t i = 0; i < query->resultCount; i++) {
        if (query->results[i] == obj1) {
            foundObj1 = true;
            break;
        }
    }
    assert(foundObj1);
    
    // Cleanup
    spatial_query_destroy(query);
    spatial_grid_destroy(grid);
    scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Spatial queries test passed\n");
}

void test_object_movement(void) {
    component_registry_init();
    transform_component_register();
    
    Scene* scene = scene_create("MovementTest", 10);
    SpatialGrid* grid = spatial_grid_create(64, 10, 10, 0, 0, 100);
    
    GameObject* obj = game_object_create(scene);
    game_object_set_position(obj, 100, 100); // Cell (1, 1)
    
    spatial_grid_add_object(grid, obj);
    
    // Move object to different cell
    game_object_set_position(obj, 300, 300); // Cell (4, 4)
    bool result = spatial_grid_update_object(grid, obj);
    assert(result == true);
    
    // Verify object is in new location
    SpatialQuery* query = spatial_query_create(10);
    query->includeStatic = true;
    uint32_t found = spatial_grid_query_circle(grid, 300, 300, 32, query);
    assert(found == 1);
    assert(query->results[0] == obj);
    
    // Verify object is not in old location
    found = spatial_grid_query_circle(grid, 100, 100, 32, query);
    assert(found == 0);
    
    spatial_query_destroy(query);
    spatial_grid_destroy(grid);
    scene_destroy(scene);
    component_registry_shutdown();
    printf("✓ Object movement test passed\n");
}

void test_world_to_cell_conversion(void) {
    SpatialGrid* grid = spatial_grid_create(64, 10, 10, 0, 0, 100);
    
    uint32_t cellX, cellY;
    
    // Test basic conversion
    bool result = spatial_grid_world_to_cell(grid, 100, 100, &cellX, &cellY);
    assert(result == true);
    assert(cellX == 1);
    assert(cellY == 1);
    
    // Test edge case
    result = spatial_grid_world_to_cell(grid, 0, 0, &cellX, &cellY);
    assert(result == true);
    assert(cellX == 0);
    assert(cellY == 0);
    
    // Test out of bounds
    result = spatial_grid_world_to_cell(grid, 1000, 1000, &cellX, &cellY);
    assert(result == false);
    
    spatial_grid_destroy(grid);
    printf("✓ World to cell conversion test passed\n");
}

#ifdef TEST_STANDALONE
int main(void) {
    printf("Running spatial grid tests...\n\n");
    
    test_spatial_grid_creation();
    test_world_to_cell_conversion();
    test_object_addition_removal();
    test_spatial_queries();
    test_object_movement();
    
    printf("\n✓ All spatial grid tests passed!\n");
    return 0;
}
#endif