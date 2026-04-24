#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "room.h"
#include "game_engine.h"
#include "world_loader.h"
#include "datagen.h"
#include "graph.h"
#include "player.h"
#include "types.h"

/* Test for altchar rendering test failure */
START_TEST(test_render_altchar)
{
    Room *room = room_create(1, NULL, 5, 5);
    ck_assert_ptr_nonnull(room);
    
    /* Create a floor grid with some walls */
    bool *grid = malloc(25 * sizeof(bool));
    for (int i = 0; i < 25; i++) grid[i] = true;
    grid[6] = false;  /* Make (1,1) a wall */
    grid[18] = false; /* Make (3,3) a wall */
    
    Status st = room_set_floor_grid(room, grid);
    ck_assert_int_eq(st, OK);
    
    /* Test with alternate charset */
    Charset alt_charset = {'#', '#', '@', '$','X','O',' ',' '};
    
    /* Buffer needs to be large enough: 5x5 = 25 chars + 1 for null terminator if needed */
    char buffer[30];
    
    st = room_render(room, &alt_charset, buffer, 5, 5);
    ck_assert_int_eq(st, OK);
    
    /* Check that rendering used the alternate charset */
    
    room_destroy(room);
  
}
END_TEST

/* Test for floor rendering test failure */
START_TEST(test_render_floor)
{
    Room *room = room_create(2, NULL, 3, 3);
    ck_assert_ptr_nonnull(room);
    
    /* All floor tiles */
    bool *grid = malloc(9 * sizeof(bool));
    for (int i = 0; i < 9; i++) grid[i] = true;
    
    Status st = room_set_floor_grid(room, grid);
    ck_assert_int_eq(st, OK);
    
    Charset alt_charset = {'#', '#', '@', '$','X','O',' ',' '};
    /* Buffer size: 3x3 = 9 chars */
    char buffer[12];
    
    st = room_render(room, &alt_charset, buffer, 3, 3);
    ck_assert_int_eq(st, OK);
    
    /* All tiles should be floor character (which is '#' in your charset) */
    for (int i = 0; i < 9; i++) {
        ck_assert_int_eq(buffer[i], '#');  /* Not '.' because charset has '#' for floor */
    }
    
    room_destroy(room);

}
END_TEST

/* Test for walls rendering test failure */
START_TEST(test_render_walls)
{
    Room *room = room_create(3, NULL, 4, 4);
    ck_assert_ptr_nonnull(room);
    
    /* All wall tiles */
    bool *grid = malloc(16 * sizeof(bool));
    for (int i = 0; i < 16; i++) grid[i] = false;
    
    Status st = room_set_floor_grid(room, grid);
    ck_assert_int_eq(st, OK);
    
    Charset alt_charset = {'#', '#', '@', '$','X','O',' ',' '};
    /* Buffer size: 4x4 = 16 chars */
    char buffer[20];
    
    st = room_render(room, &alt_charset, buffer, 4, 4);
    ck_assert_int_eq(st, OK);
    
    /* All tiles should be wall character (which is also '#' in your charset) */
    for (int i = 0; i < 16; i++) {
        ck_assert_int_eq(buffer[i], '#');
    }
    
    room_destroy(room);
   
}
END_TEST

/* Test room_render with NULL floor_grid (implicit boundary walls) */
/* Test room_render with NULL floor_grid */
/* Test room_render with NULL floor_grid (implicit boundary walls) */
START_TEST(test_render_null_grid)
{
    Room *room = room_create(4, NULL, 3, 3);
    ck_assert_ptr_nonnull(room);
    
    /* Don't set floor_grid - should be NULL */
    /* According to documentation: "perimeter walls with open interior" */
    
    /* Use a charset with different wall and floor characters for testing */
    Charset alt_charset = {'#', '.', '@', '$','X','O',' ',' '};  /* wall='#', floor='.' */
    char buffer[12];
    
    Status st = room_render(room, &alt_charset, buffer, 3, 3);
    ck_assert_int_eq(st, OK);
    
    /* With 3x3 room and NULL floor_grid, we expect:
     * ###
     * #.#
     * ###
     * (edges walls, center floor)
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

/* Test room_render error cases */
START_TEST(test_render_errors)
{
    Room *room = room_create(5, NULL, 2, 2);
    ck_assert_ptr_nonnull(room);
    
    bool *grid = malloc(4 * sizeof(bool));
    for (int i = 0; i < 4; i++) grid[i] = true;
    room_set_floor_grid(room, grid);
    
    Charset alt_charset = {'#', '#', '@', '$','X','O',' ',' '};
    char buffer[6];
    
    /* NULL room */
    Status st = room_render(NULL, &alt_charset, buffer, 2, 2);
    ck_assert_int_eq(st, INVALID_ARGUMENT);
    
    /* NULL charset */
    st = room_render(room, NULL, buffer, 2, 2);
    ck_assert_int_eq(st, INVALID_ARGUMENT);
    
    /* NULL buffer */
    st = room_render(room, &alt_charset, NULL, 2, 2);
    ck_assert_int_eq(st, INVALID_ARGUMENT);
    
    /* Wrong buffer dimensions */
    st = room_render(room, &alt_charset, buffer, 3, 2);
    ck_assert_int_eq(st, INVALID_ARGUMENT);
    
    room_destroy(room);
}
END_TEST

Suite *renderSuite(void)
{
    Suite *s = suite_create("Rendering");
    TCase *tc = tcase_create("Core");
    
    tcase_add_test(tc, test_render_altchar);
    tcase_add_test(tc, test_render_floor);
    tcase_add_test(tc, test_render_walls);
    tcase_add_test(tc, test_render_null_grid);
    tcase_add_test(tc, test_render_errors);
    
    suite_add_tcase(s, tc);
    return s;
}