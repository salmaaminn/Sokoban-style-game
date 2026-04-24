#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "room.h"

/* Test fixture - common setup for tests */
static Room *r = NULL;

static void setup(void)
{
    /* Create a basic room for testing */
    r = room_create(1, "Test Room", 10, 10);
    ck_assert_ptr_nonnull(r);
    
    /* Don't set floor_grid - let it be NULL for now */
    /* Tests will set it if needed */
}

static void teardown(void)
{
    if (r) {
        room_destroy(r);
    }
    r = NULL;
}

/* ============================================================
 * DIMENSION TESTS
 * ============================================================ */
START_TEST(test_room_dimensions_basic)
{
    Room *room = room_create(1, "Test Room", 10, 5);
    ck_assert_ptr_nonnull(room);
    
    ck_assert_int_eq(room_get_width(room), 10);
    ck_assert_int_eq(room_get_height(room), 5);
    
    room_destroy(room);
}
END_TEST

START_TEST(test_room_dimensions_null)
{
    /* Test with NULL room */
    ck_assert_int_eq(room_get_width(NULL), 0);
    ck_assert_int_eq(room_get_height(NULL), 0);
}
END_TEST

/* ============================================================
 * RENDERING TESTS
 * ============================================================ */
START_TEST(test_render_dimensions_simple)
{
    /* Create a simple 3x3 room */
    Room *room = room_create(1, NULL, 3, 3);
    ck_assert_ptr_nonnull(room);
    
    /* Allocate and set floor grid */
    bool *grid = malloc(9 * sizeof(bool));
    ck_assert_ptr_nonnull(grid);
    for (int i = 0; i < 9; i++) grid[i] = true;
    
    Status st = room_set_floor_grid(room, grid);
    ck_assert_int_eq(st, OK);
    
    /* Test rendering */
    Charset charset = {'#', '.', '@', '$','X','O',' ',' '};
    char buffer[9];  /* 3x3 = 9 chars */
    
    st = room_render(room, &charset, buffer, 3, 3);
    ck_assert_int_eq(st, OK);
    
    /* Check buffer was filled */
    for (int i = 0; i < 9; i++) {
        ck_assert_int_eq(buffer[i], '.');  /* All floors */
    }
    
    room_destroy(room);  /* This will free the grid */
}
END_TEST

START_TEST(test_render_invalid_dimensions)
{
    Room *room = room_create(1, NULL, 4, 4);
    ck_assert_ptr_nonnull(room);
    
    bool *grid = malloc(16 * sizeof(bool));
    ck_assert_ptr_nonnull(grid);
    for (int i = 0; i < 16; i++) grid[i] = true;
    
    Status st = room_set_floor_grid(room, grid);
    ck_assert_int_eq(st, OK);
    
    Charset charset = {'#', '#', '@', '$','X','O',' ',' '};
    char buffer[16];
    
    /* Wrong width */
    st = room_render(room, &charset, buffer, 3, 4);
    ck_assert_int_eq(st, INVALID_ARGUMENT);
    
    /* Wrong height */
    st = room_render(room, &charset, buffer, 4, 3);
    ck_assert_int_eq(st, INVALID_ARGUMENT);
    
    /* Correct dimensions */
    st = room_render(room, &charset, buffer, 4, 4);
    ck_assert_int_eq(st, OK);
    
    room_destroy(room);
}
END_TEST

START_TEST(test_render_walls_and_floors_simple)
{
    Room *room = room_create(1, NULL, 2, 2);
    ck_assert_ptr_nonnull(room);
    
    /* Create checkerboard pattern */
    bool *grid = malloc(4 * sizeof(bool));
    ck_assert_ptr_nonnull(grid);
    grid[0] = true;   /* (0,0) floor */
    grid[1] = false;  /* (1,0) wall */
    grid[2] = false;  /* (0,1) wall */
    grid[3] = true;   /* (1,1) floor */
    
    Status st = room_set_floor_grid(room, grid);
    ck_assert_int_eq(st, OK);
    
    Charset charset = {'#', '.', '@', '$','X','O',' ',' '};
    char buffer[4];
    
    st = room_render(room, &charset, buffer, 2, 2);
    ck_assert_int_eq(st, OK);
    
    /* Check pattern: (0,0)=., (1,0)=#, (0,1)=#, (1,1)=. */
    /* Buffer indices: 0:(0,0), 1:(1,0), 2:(0,1), 3:(1,1) */
    ck_assert_int_eq(buffer[0], '.');  /* y=0, x=0 */
    ck_assert_int_eq(buffer[1], '#');  /* y=0, x=1 */
    ck_assert_int_eq(buffer[2], '#');  /* y=1, x=0 */
    ck_assert_int_eq(buffer[3], '.');  /* y=1, x=1 */
    
    room_destroy(room);
}
END_TEST

/*
START_TEST(test_render_with_treasures_simple)
{
    Room *room = room_create(1, NULL, 3, 3);
    ck_assert_ptr_nonnull(room);
    
    // All floors
    bool *grid = malloc(9 * sizeof(bool));
    for (int i = 0; i < 9; i++) grid[i] = true;
    room_set_floor_grid(room, grid);
    
    // Create and add a treasure
    Treasure treasure = {
        .id = 42,
        .name = NULL,
        .starting_room_id = 1,
        .initial_x = 1,
        .initial_y = 1,
        .x = 1,
        .y = 1,
        .collected = false
    };
    
    Status st = room_place_treasure(room, &treasure);
    ck_assert_int_eq(st, OK);
    
    Charset charset = {'#', '.', '@', '$', 'X', 'O'};
    char buffer[9];
    
    st = room_render(room, &charset, buffer, 3, 3);
    ck_assert_int_eq(st, OK);
    
    // Treasure should be at position (1,1) which is index 4
    ck_assert_int_eq(buffer[4], '$');  // Treasure at (1,1)
    
    free(grid);
    room_destroy(room);
}
END_TEST
*/

START_TEST(test_render_null_grid_perimeter_walls)
{
    Room *room = room_create(1, NULL, 3, 3);
    ck_assert_ptr_nonnull(room);
    
    /* Don't set floor_grid - should be NULL */
    Charset charset = {'#', '.', '@', '$','X','O',' ',' '};
    char buffer[9];
    
    Status st = room_render(room, &charset, buffer, 3, 3);
    ck_assert_int_eq(st, OK);
    
    /* With NULL floor_grid, should have perimeter walls */
    /* Expected: ###
     *           #.#  
     *           ###
     */
    char expected[9] = {
        '#', '#', '#',
        '#', '.', '#',
        '#', '#', '#'
    };
    
    for (int i = 0; i < 9; i++) {
        ck_assert_int_eq(buffer[i], expected[i]);
    }
    
    room_destroy(room);
}
END_TEST

START_TEST(test_render_null_args)
{
    Room *room = room_create(1, NULL, 2, 2);
    ck_assert_ptr_nonnull(room);
    
    bool *grid = malloc(4 * sizeof(bool));
    ck_assert_ptr_nonnull(grid);
    for (int i = 0; i < 4; i++) grid[i] = true;
    
    Status st = room_set_floor_grid(room, grid);
    ck_assert_int_eq(st, OK);
    
    Charset charset = {'#', '#', '@', '$','X','O',' ',' '};
    char buffer[4];
    
    /* NULL room */
    st = room_render(NULL, &charset, buffer, 2, 2);
    ck_assert_int_eq(st, INVALID_ARGUMENT);
    
    /* NULL charset */
    st = room_render(room, NULL, buffer, 2, 2);
    ck_assert_int_eq(st, INVALID_ARGUMENT);
    
    /* NULL buffer */
    st = room_render(room, &charset, NULL, 2, 2);
    ck_assert_int_eq(st, INVALID_ARGUMENT);
    
    room_destroy(room);
}
END_TEST
//////////A2//////////////////////////////////
START_TEST(test_room_get_id) {
    ck_assert_int_eq(room_get_id(r), 1);
}
END_TEST

START_TEST(test_room_get_id_null) {
    ck_assert_int_eq(room_get_id(NULL), -1);
}
END_TEST


START_TEST(test_room_is_walkable_floor) {
    ck_assert(room_is_walkable(r, 2, 2));
}
END_TEST

START_TEST(test_room_is_walkable_wall) {
    ck_assert(!room_is_walkable(r, 0, 0));
}
END_TEST

START_TEST(test_room_is_walkable_out_of_bounds) {
    ck_assert(!room_is_walkable(r, -1, 0));
    ck_assert(!room_is_walkable(r, 10, 10));  // 10 is out of bounds for a 10x10 room
    ck_assert(!room_is_walkable(r, 100, 100));
}
END_TEST

START_TEST(test_room_is_walkable_null) {
    ck_assert(!room_is_walkable(NULL, 2, 2));
}
END_TEST

START_TEST(test_room_is_walkable_with_pushable) {
    r->pushables = calloc(1, sizeof(Pushable));
    r->pushable_count = 1;
    r->pushables[0].x = 2;
    r->pushables[0].y = 1;
    ck_assert(!room_is_walkable(r, 2, 1));
}
END_TEST

START_TEST(test_room_pick_up_treasure) {
    r->treasures = calloc(1, sizeof(Treasure));
    r->treasure_count = 1;
    r->treasures[0].id = 10;
    r->treasures[0].collected = false;

    Treasure *t = NULL;
    ck_assert_int_eq(room_pick_up_treasure(r, 10, &t), OK);
    ck_assert_ptr_nonnull(t);
    //ck_assert(r->treasures[0].collected);
}
END_TEST

START_TEST(test_room_pick_up_treasure_not_found) {
    r->treasures = calloc(1, sizeof(Treasure));
    r->treasure_count = 1;
    r->treasures[0].id = 10;
    r->treasures[0].collected = false;

    Treasure *t = NULL;
    ck_assert_int_eq(room_pick_up_treasure(r, 999, &t), ROOM_NOT_FOUND);
}
END_TEST

START_TEST(test_room_pick_up_treasure_already_collected) {
    r->treasures = calloc(1, sizeof(Treasure));
    r->treasure_count = 1;
    r->treasures[0].id = 10;
    r->treasures[0].collected = true;

    Treasure *t = NULL;
    ck_assert_int_ne(room_pick_up_treasure(r, 10, &t), OK);
}
END_TEST

START_TEST(test_room_pick_up_treasure_null_room) {
    Treasure *t = NULL;
    ck_assert_int_eq(room_pick_up_treasure(NULL, 10, &t), INVALID_ARGUMENT);
}
END_TEST

START_TEST(test_room_pick_up_treasure_null_out) {
    r->treasures = calloc(1, sizeof(Treasure));
    r->treasure_count = 1;
    r->treasures[0].id = 10;

    ck_assert_int_eq(room_pick_up_treasure(r, 10, NULL), INVALID_ARGUMENT);
}
END_TEST

START_TEST(test_room_pick_up_treasure_all) {
    r->treasures = calloc(2, sizeof(Treasure));
    r->treasure_count = 2;
    r->treasures[0].id = 1;
    r->treasures[1].id = 2;

    Treasure *t1 = NULL, *t2 = NULL;
    ck_assert_int_eq(room_pick_up_treasure(r, 1, &t1), OK);
    ck_assert_int_eq(room_pick_up_treasure(r, 2, &t2), OK);
    ck_assert_ptr_nonnull(t1);
    ck_assert_ptr_nonnull(t2);
}
END_TEST

START_TEST(test_room_try_push_invalid_index)
{
    r->pushables = calloc(1, sizeof(Pushable));
    r->pushable_count = 1;
    r->pushables[0].x = 2;
    r->pushables[0].y = 2;

    /* index 1 is out of bounds for a 1-pushable room */
    ck_assert_int_eq(room_try_push(r, 1, DIR_NORTH), INVALID_ARGUMENT);
}
END_TEST
START_TEST(test_room_classify_tile_pushable)
{
    r->pushables = calloc(1, sizeof(Pushable));
    r->pushable_count = 1;
    r->pushables[0].id = 0;
    r->pushables[0].x = 2;
    r->pushables[0].y = 2;

    int out_id = -1;
    RoomTileType type = room_classify_tile(r, 2, 2, &out_id);
    ck_assert_int_eq(type, ROOM_TILE_PUSHABLE);
    ck_assert_int_eq(out_id, 0);  
}
END_TEST
Suite *roomSuite(void) {
    Suite *s = suite_create("Room");
    TCase *tc_core = tcase_create("Core");
    TCase *tc_render = tcase_create("Rendering");
    TCase *tc_walkable = tcase_create("Walkable");
    TCase *tc_treasure = tcase_create("Treasure");

    /* Existing core tests */
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_room_dimensions_basic);
    tcase_add_test(tc_core, test_room_dimensions_null);
    tcase_add_test(tc_core, test_room_get_id);
    tcase_add_test(tc_core, test_room_get_id_null);

    /* Existing render tests */
    tcase_add_test(tc_render, test_render_dimensions_simple);
    tcase_add_test(tc_render, test_render_invalid_dimensions);
    tcase_add_test(tc_render, test_render_walls_and_floors_simple);
    tcase_add_test(tc_render, test_render_null_grid_perimeter_walls);
    tcase_add_test(tc_render, test_render_null_args);

    /* New walkable tests */
    tcase_add_checked_fixture(tc_walkable, setup, teardown);
    tcase_add_test(tc_walkable, test_room_is_walkable_floor);
    tcase_add_test(tc_walkable, test_room_is_walkable_wall);
    tcase_add_test(tc_walkable, test_room_is_walkable_out_of_bounds);
    tcase_add_test(tc_walkable, test_room_is_walkable_null);
    tcase_add_test(tc_walkable, test_room_is_walkable_with_pushable);
    tcase_add_test(tc_walkable, test_room_classify_tile_pushable);
    tcase_add_test(tc_walkable, test_room_try_push_invalid_index);

    /* New treasure tests */
    tcase_add_checked_fixture(tc_treasure, setup, teardown);
    tcase_add_test(tc_treasure, test_room_pick_up_treasure);
    tcase_add_test(tc_treasure, test_room_pick_up_treasure_not_found);
    tcase_add_test(tc_treasure, test_room_pick_up_treasure_already_collected);
    tcase_add_test(tc_treasure, test_room_pick_up_treasure_null_room);
    tcase_add_test(tc_treasure, test_room_pick_up_treasure_null_out);
    tcase_add_test(tc_treasure, test_room_pick_up_treasure_all);

    suite_add_tcase(s, tc_core);
    suite_add_tcase(s, tc_render);
    suite_add_tcase(s, tc_walkable);
    suite_add_tcase(s, tc_treasure);
    return s;
}