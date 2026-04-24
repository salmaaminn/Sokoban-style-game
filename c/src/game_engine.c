#include "game_engine.h"
#include <stdlib.h>
#include "room.h"
#include "graph.h"
#include "player.h"
#include "world_loader.h"
#include <string.h>
#include "total_treasure.h"
extern int total_treasure;
Status game_engine_create(const char *config_file_path, GameEngine **engine_out) {
    if (config_file_path == NULL || engine_out == NULL) {
        return INVALID_ARGUMENT;
    }
    *engine_out = NULL;

    GameEngine *engine = malloc(sizeof(GameEngine));
    if (engine == NULL) {
        return NO_MEMORY;
    }

    Graph *graph = NULL;
    Room *first_room = NULL;
    int room_count = 0;
    Charset charset;

    Status status_l = loader_load_world(config_file_path, &graph, &first_room, &room_count, &charset);
    if (status_l != OK) {
        free(engine);
        return status_l;
    }

    engine->graph = graph;
    engine->room_count = room_count;
    engine->charset = charset;

    int start_x = 0;
    int start_y = 0;
    int start_room_id = 0; 

    if (first_room != NULL) {
        start_room_id = first_room->id; // room id
        room_get_start_position(first_room, &start_x, &start_y);
    }

    engine->initial_room_id = start_room_id;
    engine->initial_player_x = start_x; //initilaize pos
    engine->initial_player_y = start_y;

    if (player_create(start_room_id, start_x, start_y, &engine->player) != OK) {
        graph_destroy(engine->graph);
        free(engine);
        return NO_MEMORY;
    }
    *engine_out = engine;
    return OK;
}

void game_engine_destroy(GameEngine *eng) {
    if (!eng) { return; }
    player_destroy(eng->player);
    graph_destroy(eng->graph);
    free(eng);
}

const Player *game_engine_get_player(const GameEngine *eng) {
    if (eng == NULL) { return NULL; }
    return eng->player;
}

static void direction_to_delta(Direction dir, int *dx, int *dy) {
    if (dir == DIR_NORTH)      { *dy = -1; }
    else if (dir == DIR_SOUTH) { *dy =  1; }
    else if (dir == DIR_EAST)  { *dx =  1; }
    else if (dir == DIR_WEST)  { *dx = -1; }
}

static bool portal_is_unlocked(const Room *room, int px, int py) {
    for (int i = 0; i < room->portal_count; i++) {
        if (room->portals[i].x != px || room->portals[i].y != py) { continue; }
        if (!room->portals[i].gated) { return true; }
        for (int j = 0; j < room->switch_count; j++) {
            if (room->switches[j].portal_id != room->portals[i].required_switch_id) { continue; }
            for (int k = 0; k < room->pushable_count; k++) {
                if (room->pushables[k].x == room->switches[j].x &&
                    room->pushables[k].y == room->switches[j].y) {
                    return true;
                }
            }
            return false;
        }
        return false;
    }
    return true;
}

static Status treasure_tile(GameEngine *eng, Room *room, int out_id) {
    Treasure *t = NULL;
    room_pick_up_treasure(room, out_id, &t);
    if (t != NULL) {
        player_try_collect(eng->player, t);
    }
    return OK;
}

static Status pushable_tile(GameEngine *eng, Room *room,  Direction dir, int out_id, int newx, int newy) {
    Status push = room_try_push(room, out_id, dir);
    if (push != OK) { return ROOM_IMPASSABLE; }
    player_set_position(eng->player, newx, newy);
    return OK;
}

static Status portal_tile(GameEngine *eng, Room *room,
                           int newx, int newy, int out_id) {
    if (out_id == -1) {
        player_set_position(eng->player, newx, newy);
        return OK;
    }
    Room r_key = { .id = out_id };
    Room *next = (Room *)graph_get_payload(eng->graph, &r_key);
    if (next == NULL) {
        player_set_position(eng->player, newx, newy);
        return OK;
    }
    if (!portal_is_unlocked(room, newx, newy)) {
        return ROOM_IMPASSABLE;
    }
    player_set_position(eng->player, newx, newy);
    player_move_to_room(eng->player, out_id);
    int sx = 0;
    int sy = 0;
    room_get_start_position(next, &sx, &sy);
    player_set_position(eng->player, sx, sy);
    return OK;
}
//moving player
Status game_engine_move_player(GameEngine *eng, Direction dir) {
    if (eng == NULL || eng->player == NULL) { return INVALID_ARGUMENT; }
    //look up current room of player
    Room cur = { .id = eng->player->room_id };
    Room *room = (Room *)graph_get_payload(eng->graph, &cur);
    if (!room) { return GE_NO_SUCH_ROOM; }

    int dx = 0;
    int dy = 0;
    direction_to_delta(dir, &dx, &dy);
    if (dx == 0 && dy == 0) { return INVALID_ARGUMENT; }

    int newx = eng->player->x + dx;
    int newy = eng->player->y + dy;

    int out_id = -1;
    //check tile type
    RoomTileType tile = room_classify_tile(room, newx, newy, &out_id); //tile index

    if (tile == ROOM_TILE_INVALID || tile == ROOM_TILE_WALL) { return ROOM_IMPASSABLE; }
    if (tile == ROOM_TILE_TREASURE) { return treasure_tile(eng, room, out_id); }
    if (tile == ROOM_TILE_PUSHABLE) { return pushable_tile(eng, room, dir, out_id, newx, newy); }
    if (tile == ROOM_TILE_PORTAL)   { return portal_tile(eng, room, newx, newy, out_id); }
    
    player_set_position(eng->player, newx, newy);
    return OK;
}

Status game_engine_get_room_count(const GameEngine *eng, int *count_out) {
    if (!eng) { return INVALID_ARGUMENT; }
    if (!count_out) { return NULL_POINTER; }
    *count_out = eng->room_count;
    return OK;
}

Status game_engine_get_room_dimensions(const GameEngine *eng,
                                       int *width_out, int *height_out) {
    if (!eng) { return INVALID_ARGUMENT; }
    if (!width_out || !height_out) { return NULL_POINTER; }
    if (!eng->player || !eng->graph) { return INTERNAL_ERROR; }

    Room key = { .id = eng->player->room_id };
    Room *room = (Room *)graph_get_payload(eng->graph, &key);
    if (!room) { return GE_NO_SUCH_ROOM; }

    *width_out  = room_get_width(room);
    *height_out = room_get_height(room);
    return OK;
}

Status game_engine_reset(GameEngine *eng) {
    if (!eng || !eng->player) { return INVALID_ARGUMENT; }
    return player_reset_to_start(eng->player,
                                 eng->initial_room_id,
                                 eng->initial_player_x,
                                 eng->initial_player_y);
}

static Status render_room_to_string(const Room *room, const Charset *charset,
                                    char **str_out) {
    int w = room_get_width(room);
    int h = room_get_height(room);

    char *buf = malloc((size_t)(w + 1) * (size_t)h + 1);
    if (!buf) { return NO_MEMORY; }

    char *temp_buf = malloc((size_t)w * (size_t)h);
    if (!temp_buf) { free(buf); return NO_MEMORY; }

    if (room_render(room, charset, temp_buf, w, h) != OK) {
        free(temp_buf);
        free(buf);
        return INTERNAL_ERROR;
    }

    for (int row = 0; row < h; row++) {
        memcpy(&buf[(size_t)row * (size_t)(w + 1)],
               &temp_buf[(size_t)row * (size_t)w], (size_t)w);
        buf[row * (w + 1) + w] = '\n';
    }
    buf[(size_t)h * (size_t)(w + 1)] = '\0';

    free(temp_buf);
    *str_out = buf;
    return OK;
}

Status game_engine_render_current_room(const GameEngine *eng, char **str_out) {
    if (!eng || !eng->player || !str_out) { return INVALID_ARGUMENT; }

    Room key = { .id = eng->player->room_id };
    Room *room = (Room *)graph_get_payload(eng->graph, &key);
    if (!room) { return GE_NO_SUCH_ROOM; }

    Status st = render_room_to_string(room, &eng->charset, str_out);
    if (st != OK) { return st; }

    int player_index = eng->player->y * (room_get_width(room) + 1) + eng->player->x;
    (*str_out)[player_index] = eng->charset.player;
    return OK;
}

Status game_engine_render_room(const GameEngine *eng, int room_id, char **str_out) {
    if (!eng || !str_out) { return INVALID_ARGUMENT; }

    Room key = { .id = room_id };
    Room *room = (Room *)graph_get_payload(eng->graph, &key);
    if (!room) { return GE_NO_SUCH_ROOM; }

    return render_room_to_string(room, &eng->charset, str_out);
}

Status game_engine_get_room_ids(const GameEngine *eng,
                                int **ids_out, int *count_out) {
    if (!eng) { return INVALID_ARGUMENT; }
    if (!ids_out || !count_out) { return NULL_POINTER; }

    int n = eng->room_count;
    *count_out = n;

    int *arr = malloc(sizeof(int) * (size_t)n);
    if (!arr) { return NO_MEMORY; }

    const void * const *payloads = NULL;
    int actual_count = 0;
    if (graph_get_all_payloads(eng->graph, &payloads, &actual_count) != GRAPH_STATUS_OK) {
        free(arr);
        return INTERNAL_ERROR;
    }

    for (int i = 0; i < n && i < actual_count; i++) {
        arr[i] = ((Room *)payloads[i])->id;
    }

    *ids_out = arr;
    return OK;
}

void game_engine_free_string(void *ptr) {
    free(ptr);
}

int game_engine_get_total_treasures(void) {
    return total_treasure;
}
///////////
int game_engine_total_treasures(GameEngine *eng) {
    int total_treasure_count = game_engine_get_total_treasures();
    int player_treasure_collected = player_get_collected_count(eng->player);
    if (player_treasure_collected == total_treasure_count){
        //printf("All treasures collected! Total treasures: %d, Total collected: %d\n", total_treasure_count, player_treasure_collected);
        return 1;
    }
    //printf("All treasures not collected :( Total treasures: %d, Total collected: %d\n", total_treasure_count, player_treasure_collected);
    return 0;
}
///////////