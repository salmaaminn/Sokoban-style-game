#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include "game_engine.h"
#include "datagen.h"
#include "room.h"
#include "graph.h"
#include "player.h"



static GameEngine *eng = NULL;


static void setup_engine(void)
{
    Status st = game_engine_create("../assets/starter.ini", &eng);

    //printf("st1=%d st2=%d\n", st1, st2);
     //printf("create status: %d, eng: %p\n", st, (void*)eng);
     if (st != OK) {
        eng = NULL;  
    }
    
}

static void teardown_engine(void)
{
    game_engine_destroy(eng);
    eng = NULL;
}

START_TEST(test_engine_create_destroy)
{
    ck_assert_ptr_nonnull(eng);
}
END_TEST

START_TEST(test_engine_move_all_directions)
{
    ck_assert_ptr_nonnull(eng);
    
    Status move_st;
    const Player *player = game_engine_get_player(eng);
    int start_x, start_y;
    player_get_position(player, &start_x, &start_y);
    
    
        
    
    move_st = game_engine_move_player(eng, DIR_NORTH);
   
    ck_assert(move_st == OK || move_st == ROOM_IMPASSABLE);
    
    /* Debug reset */
    Status reset_st = game_engine_reset(eng);
    ck_assert_int_eq(reset_st, OK);
    
    /* Check player after reset */
    player = game_engine_get_player(eng);
    player_get_position(player, &start_x, &start_y);
   
   
    
    move_st = game_engine_move_player(eng, DIR_SOUTH);
   
    ck_assert(move_st == OK || move_st == ROOM_IMPASSABLE);
}

END_TEST

START_TEST(test_engine_move_invalid_args)
{
    ck_assert_ptr_nonnull(eng);
    Status move_st = game_engine_move_player(NULL, DIR_NORTH);
    ck_assert_int_eq(move_st, INVALID_ARGUMENT);
    move_st = game_engine_move_player(eng, (Direction)99);
    ck_assert_int_eq(move_st, INVALID_ARGUMENT);
}
END_TEST

START_TEST(test_engine_move_sequence)
{
    ck_assert_ptr_nonnull(eng);
    const Player *player = game_engine_get_player(eng);
    int start_x, start_y;
    player_get_position(player, &start_x, &start_y);
    
    Status move_st;
    int success_count = 0;
    
    move_st = game_engine_move_player(eng, DIR_NORTH);
    if (move_st == OK) success_count++;
    
    move_st = game_engine_move_player(eng, DIR_EAST);
    if (move_st == OK) success_count++;
    
    move_st = game_engine_move_player(eng, DIR_SOUTH);
    if (move_st == OK) success_count++;
    
    move_st = game_engine_move_player(eng, DIR_WEST);
    if (move_st == OK) success_count++;
    
    ck_assert_int_gt(success_count, 0);
}
END_TEST
////a2////////////////////////////////

//
START_TEST(test_engine_create_multiple_times)
{
    GameEngine *eng1 = NULL;
    GameEngine *eng2 = NULL;
    Status st1 = game_engine_create("../assets/starter.ini", &eng1);
    Status st2 = game_engine_create("../assets/starter.ini", &eng2);
    ck_assert_ptr_nonnull(eng1);
    ck_assert_ptr_nonnull(eng2);
    ck_assert_int_eq(st1, OK);
    ck_assert_int_eq(st2, OK);
    game_engine_destroy(eng1);
    game_engine_destroy(eng2);
}
END_TEST
//

START_TEST(test_engine_get_player)
{
    ck_assert_ptr_nonnull(eng);
    const Player *p = game_engine_get_player(eng);
    ck_assert_ptr_nonnull(p);
    int x, y;
    Status pos_st = player_get_position(p, &x, &y);
    ck_assert_int_eq(pos_st, OK);
    ck_assert_int_ge(x, 0);
    ck_assert_int_ge(y, 0);
}
END_TEST


START_TEST(test_engine_move_and_back)
{
    ck_assert_ptr_nonnull(eng);

    Direction dirs[] = { DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST };
    Direction opposites[] = { DIR_SOUTH, DIR_NORTH, DIR_WEST, DIR_EAST };

    int found = 0;
    for (int i = 0; i < 4; i++) {
        game_engine_reset(eng);
        Status st1 = game_engine_move_player(eng, dirs[i]);
        if (st1 == OK) {
            Status st2 = game_engine_move_player(eng, opposites[i]);
            if (st2 == OK) {
                found = 1;
                break;
            }
        }
    }
    ck_assert_msg(found, "Could not find direction allowing 2 consecutive moves");
}
END_TEST
Suite *game_engineSuite(void)
{
    Suite *s = suite_create("GameEngine");
    TCase *tc = tcase_create("Core");
    tcase_add_checked_fixture(tc, setup_engine, teardown_engine);
    tcase_add_test(tc, test_engine_create_destroy);
    tcase_add_test(tc, test_engine_move_all_directions);
    tcase_add_test(tc, test_engine_move_invalid_args);
    tcase_add_test(tc, test_engine_move_sequence);

    
    tcase_add_test(tc, test_engine_create_multiple_times);
    tcase_add_test(tc, test_engine_get_player);
    tcase_add_test(tc, test_engine_move_and_back);

    suite_add_tcase(s, tc);
    return s;
}