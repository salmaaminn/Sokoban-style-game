#include <check.h>
#include <stdlib.h>
#include "player.h"

/* ============================================================
 * Setup and Teardown fixtures
 * ============================================================ */

static Player *p= NULL;

static void setup_player(void)
{

    Status st = player_create(1, 5, 10, &p);
    ck_assert_int_eq(st, OK);
    ck_assert_ptr_nonnull(p);
}

static void teardown_player(void)
{
    /* graph_destroy must call destroy_int on all payloads */
    player_destroy(p);
    p = NULL;
}

/* ============================================================
 * TESTS
 * ============================================================ */

//create
START_TEST(test_playerCreate){
    ck_assert_int_eq(player_create(1, 5, 10, &p),OK);
}
END_TEST

START_TEST(test_playerNull){
    ck_assert_int_eq(player_create(1, 5, 10, NULL),INVALID_ARGUMENT);
}
END_TEST

//get room
START_TEST(test_playerNullGetRoom){
    ck_assert_int_eq(player_get_room(NULL),-1);
}
END_TEST

START_TEST(test_playerGetRoom){
    ck_assert_int_eq(player_get_room(p),p->room_id);
}
END_TEST

//get position
START_TEST(test_playerGetPositionNull){
    int x_out;
    int y_out;
    ck_assert_int_eq(player_get_position(NULL, &x_out, &y_out),INVALID_ARGUMENT);
}
END_TEST
START_TEST(test_playerGetPositionNullX){
    int x_out;
    ck_assert_int_eq(player_get_position(p, NULL, &x_out),INVALID_ARGUMENT);
}
END_TEST
START_TEST(test_playerGetPositionNullY){
    int y_out;
    ck_assert_int_eq(player_get_position(p, &y_out, NULL),INVALID_ARGUMENT);
}
END_TEST
START_TEST(test_playerGetPosition){
    int x_out;                         
    int y_out;
    ck_assert_int_eq(player_get_position(p, &x_out, &y_out),OK);
    ck_assert_int_eq(x_out, 5);
    ck_assert_int_eq(y_out, 10);
}
END_TEST

//set position
START_TEST(test_playerSetPosition){
    int x = 30;
    int y = 20;
    ck_assert_int_eq(player_set_position(p,x, y),OK);
    ck_assert(p->x == 30);
    ck_assert(p->y == 20);
}
END_TEST

START_TEST(test_playerSetPositionNull){
    int x = 30;
    int y = 20;
    ck_assert_int_eq(player_set_position(NULL,x, y),INVALID_ARGUMENT);
}
END_TEST

//move to room
START_TEST(test_playerNullMoveRoom){
    ck_assert_int_eq(player_move_to_room(NULL, 5), INVALID_ARGUMENT);
}
END_TEST
START_TEST(test_playerMoveRoom){
    ck_assert_int_eq(player_move_to_room(p, 5),OK);
    ck_assert( p->room_id == 5);
}
END_TEST

//reset player
START_TEST(test_playerReset){
    ck_assert_int_eq(player_reset_to_start(p, 1, 3, 5),OK);
    ck_assert( p->room_id == 1);
    ck_assert( p->x == 3);
    ck_assert( p->y == 5);
}
END_TEST
START_TEST(test_playerNullReset){
    ck_assert_int_eq(player_reset_to_start(NULL, 1, 3, 5), INVALID_ARGUMENT);
}

/////////A2//////////////////////////////////
START_TEST(test_playerCollectSingleTreasure){
    /* freshly created player should have no treasures */
    ck_assert_int_eq(player_get_collected_count(p), 0);

    Treasure t = {.id = 7, .collected = false};

    ck_assert_int_eq(player_try_collect(p, &t), OK);
    ck_assert_int_eq(player_get_collected_count(p), 1);
    ck_assert(player_has_collected_treasure(p, 7));

    int count;
    const Treasure * const *arr = player_get_collected_treasures(p, &count);
    ck_assert_int_eq(count, 1);
    ck_assert_ptr_eq(arr[0], &t);
    ck_assert(t.collected == true);
}
END_TEST

START_TEST(test_playerCollectNullArgs){
    Treasure t = {.id = 3, .collected = false};
    /* player or treasure pointers cannot be NULL */
    ck_assert_int_eq(player_try_collect(NULL, &t), NULL_POINTER);
    ck_assert_int_eq(player_try_collect(p, NULL), NULL_POINTER);
}
END_TEST

START_TEST(test_playerCollectDuplicate){
    Treasure t = {.id = 5, .collected = false};
    ck_assert_int_eq(player_try_collect(p, &t), OK);
    /* collecting same treasure again or different player should fail */
    ck_assert_int_eq(player_try_collect(p, &t), INVALID_ARGUMENT);
    /* manually mark collected and try from new player */
    Player *other = NULL;
    ck_assert_int_eq(player_create(0,0,0,&other), OK);
    t.collected = true; /* simulate already picked up */
    ck_assert_int_eq(player_try_collect(other, &t), 0);
    player_destroy(other);
}
END_TEST

/* ============================================================
 * SUITE CREATION
 * ============================================================ */

Suite *playerSuite(void)
{
    Suite *s = suite_create("Player");
    TCase *tc = tcase_create("Core");

    tcase_add_checked_fixture(tc, setup_player, teardown_player);

    //create 
    tcase_add_test(tc, test_playerCreate);
    tcase_add_test(tc, test_playerNull);

    //get room 
    tcase_add_test(tc, test_playerNullGetRoom);
    tcase_add_test(tc, test_playerGetRoom);

    //get position 
    tcase_add_test(tc, test_playerGetPositionNull);
    tcase_add_test(tc, test_playerGetPositionNullX);
    tcase_add_test(tc, test_playerGetPositionNullY);
    tcase_add_test(tc, test_playerGetPosition);

    //set position 
    tcase_add_test(tc, test_playerSetPosition);
    tcase_add_test(tc, test_playerSetPositionNull);

    //move to room 
    tcase_add_test(tc, test_playerNullMoveRoom);
    tcase_add_test(tc, test_playerMoveRoom);

    //reset player 
    tcase_add_test(tc, test_playerReset);
    tcase_add_test(tc, test_playerNullReset);

    /* A2 */
    tcase_add_test(tc, test_playerCollectSingleTreasure);
    tcase_add_test(tc, test_playerCollectNullArgs);
    tcase_add_test(tc, test_playerCollectDuplicate);

    suite_add_tcase(s, tc);
    return s;
}