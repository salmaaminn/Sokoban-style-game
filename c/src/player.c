#include <stdio.h>
#include "player.h"
#include <stdlib.h>
#include "total_treasure.h"

Status player_create(int initial_room_id, int initial_x, int initial_y, Player **player_out){
    if(player_out == NULL){
        return INVALID_ARGUMENT;
    }                                       
    Player *new_player = malloc(sizeof(Player));
    if(new_player == NULL){
        return NO_MEMORY;
    }
    new_player->room_id = initial_room_id;
    new_player->x = initial_x;
    new_player->y = initial_y;

    /* set collection state to empty */
    new_player->collected_treasures = NULL;
    new_player->collected_count = 0;

    *player_out = new_player;
    return OK;
    

}
//////
void player_destroy(Player *p){
    if (p == NULL) {
        return;
    }
    /* free the dynamic array of borrowed treasure pointers */
    free(p->collected_treasures);
    free(p);
}
//////
int player_get_room(const Player *p){
    if (p == NULL){
        return -1;
    }
   return p->room_id; 
}
///////
Status player_get_position(const Player *p, int *x_out, int *y_out){
 if (p == NULL || x_out == NULL || y_out == NULL){
        return INVALID_ARGUMENT;
    }
   *x_out = p->x;
   *y_out = p->y;
   return OK;
}
///////////
Status player_set_position(Player *p, int x, int y){
    if (p == NULL){
        return INVALID_ARGUMENT;
    }
    p->x = x;
    p->y = y;
    return OK;
}
//////////
Status player_move_to_room(Player *p, int new_room_id){
    if (p == NULL){
        return INVALID_ARGUMENT;
    }
    p->room_id = new_room_id;
    return OK;
}
//////////

Status player_reset_to_start(Player *p, int starting_room_id, int start_x, int start_y){
     if (p == NULL){
        return INVALID_ARGUMENT;
    }
    /* clear any previously collected treasures */
    free(p->collected_treasures);
    p->collected_treasures = NULL;
    p->collected_count = 0;

    p->room_id = starting_room_id;
    p->x = start_x;
    p->y = start_y;
    return OK;
}
////
Status player_try_collect(Player *p, Treasure *treasure){
    if(p == NULL || treasure == NULL){return NULL_POINTER;}
    /*if(treasure->collected == true){
        return INVALID_ARGUMENT;
    }*/
    for (int i = 0; i < p->collected_count; i++) {
        Treasure *tarr = p->collected_treasures[i];  // get the pointer
        if(tarr == treasure){return INVALID_ARGUMENT;}
    }
    
    Treasure **incr_array = realloc(p->collected_treasures, (p->collected_count+1) * sizeof(Treasure *));
    if (incr_array == NULL) {
        return NO_MEMORY;
    }

    p->collected_treasures = incr_array;
    p->collected_treasures[p->collected_count] = treasure;
    treasure->collected = true;
    p->collected_count++;
    return OK;
}
///
bool player_has_collected_treasure(const Player *p, int treasure_id){
    if(p == NULL || treasure_id < 0){
        return false;
    }
     for (int i = 0; i < p->collected_count; i++) {
        Treasure *tarr = p->collected_treasures[i];  // get the pointer
        if(tarr->id == treasure_id){
            return true;
        }
    }
    return false;
}
///
int player_get_collected_count(const Player *p){
    if(p == NULL){
        return 0;
    }
    return p->collected_count;
}
//////////////////////collect all treasure////////////////////////


const Treasure * const *player_get_collected_treasures(const Player *p, int *count_out){
    if(p == NULL || count_out == NULL){
        return NULL;
    }
    *count_out = p->collected_count;
    return (const Treasure * const *) p->collected_treasures;
}
///

