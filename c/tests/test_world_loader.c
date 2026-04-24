#include "player.h"
#include <check.h>
#include <stdlib.h>
#include "world_loader.h"
#include "room.h"
#include "graph.h"

START_TEST(test_loader_invalid_config_path)
{
    Graph *graph = NULL;
    Room *first_room = NULL;
    int room_count = 0;
    Charset charset;

    Status st = loader_load_world("not_a_real_file.ini", &graph, &first_room, &room_count, &charset);
    ck_assert_int_eq(st, WL_ERR_CONFIG);
}
END_TEST

START_TEST(test_loader_empty_config_path)
{
    Graph *graph = NULL;
    Room *first_room = NULL;
    int room_count = 0;
    Charset charset;

    Status st = loader_load_world("", &graph, &first_room, &room_count, &charset);
    ck_assert_int_eq(st, WL_ERR_CONFIG);
}
END_TEST

START_TEST(test_loader_nonexistent_directory)
{
    Graph *graph = NULL;
    Room *first_room = NULL;
    int room_count = 0;
    Charset charset;

    Status st = loader_load_world("/nonexistent/path/config.ini", &graph, &first_room, &room_count, &charset);
    ck_assert_int_eq(st, WL_ERR_CONFIG);
}
END_TEST



/* ============================================================
 * SUITE CREATION
 * ============================================================ */

Suite *loaderSuite(void)
{
    Suite *s = suite_create("Player");
    TCase *tc = tcase_create("Core");

    //create 
    tcase_add_test(tc, test_loader_invalid_config_path);
    tcase_add_test(tc, test_loader_empty_config_path);
    tcase_add_test(tc, test_loader_nonexistent_directory);

    suite_add_tcase(s, tc);
    return s;
}