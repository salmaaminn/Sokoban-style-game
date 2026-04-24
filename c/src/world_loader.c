#include <stdio.h>
#include "world_loader.h"
#include <stdlib.h>
#include "datagen.h"
#include "room.h"
#include "graph.h"
#include "player.h"
#include "game_engine.h"
#include <string.h>
int total_treasure = 0;
static void room_destroy_adapter(void *data) {
    room_destroy((Room *)data);
}

static int room_compare(const void *a, const void *b) {
    const Room *ra = a;
    const Room *rb = b;
    if (ra->id < rb->id) { return -1; }
    if (ra->id > rb->id) { return 1; }
    return 0;
}

static void copy_charset(const DG_Charset *dg_charset, Charset *charset_out) {
    if (dg_charset != NULL) {
        charset_out->wall     = dg_charset->wall;
        charset_out->floor    = dg_charset->floor;
        charset_out->player   = dg_charset->player;
        charset_out->treasure = dg_charset->treasure;
        charset_out->portal   = dg_charset->portal;
        charset_out->pushable = dg_charset->pushable;
    } else {
        charset_out->wall     = '#';
        charset_out->floor    = '.';
        charset_out->player   = '@';
        charset_out->treasure = '$';
        charset_out->portal   = 'X';
        charset_out->pushable = 'O';
    }
}

static Status copy_portals(Room *r, const DG_Room *dg_room) {
    if (dg_room->portal_count <= 0 || dg_room->portals == NULL) { return OK; }

    Portal *portals = malloc((size_t)dg_room->portal_count * sizeof(Portal));
    if (!portals) { return NO_MEMORY; }

    for (int i = 0; i < dg_room->portal_count; i++) {
        portals[i].id             = dg_room->portals[i].id;
        portals[i].x              = dg_room->portals[i].x;
        portals[i].y              = dg_room->portals[i].y;
        portals[i].target_room_id = dg_room->portals[i].neighbor_id;
        portals[i].name           = NULL;
        portals[i].gated             = dg_room->portals[i].required_switch_id != -1;
portals[i].required_switch_id = dg_room->portals[i].required_switch_id;
    }

    Status st = room_set_portals(r, portals, dg_room->portal_count);
    if (st != OK) {
        free(portals);
        return NO_MEMORY;
    }
    return OK;
}

static Status copy_treasures(Room *r, const DG_Room *dg_room) {
    if (dg_room->treasure_count <= 0 || dg_room->treasures == NULL) { return OK; }

    Treasure *treasures = malloc((size_t)dg_room->treasure_count * sizeof(Treasure));
    if (!treasures) { return NO_MEMORY; }
    
    for (int i = 0; i < dg_room->treasure_count; i++) {
        treasures[i].id              = dg_room->treasures[i].global_id;
        treasures[i].starting_room_id = dg_room->id;
        treasures[i].initial_x       = dg_room->treasures[i].x;
        treasures[i].initial_y       = dg_room->treasures[i].y;
        treasures[i].x               = dg_room->treasures[i].x;
        treasures[i].y               = dg_room->treasures[i].y;
        treasures[i].collected        = false;

        if (dg_room->treasures[i].name) {
            treasures[i].name = strdup(dg_room->treasures[i].name);
            if (!treasures[i].name) {
                for (int j = 0; j < i; j++) { free(treasures[j].name); }
                free(treasures);
                return NO_MEMORY;
            }
        } else {
            treasures[i].name = NULL;
        }
        
    }
    //extention
    total_treasure += dg_room->treasure_count;
    Status st = room_set_treasures(r, treasures, dg_room->treasure_count);
    if (st != OK) {
        for (int i = 0; i < dg_room->treasure_count; i++) { free(treasures[i].name); }
        free(treasures);
        return NO_MEMORY;
    }
    return OK;
}
//
static Status copy_pushables(Room *r, const DG_Room *dg_room) {
    if (dg_room->pushable_count <= 0 || dg_room->pushables == NULL) { return OK; }

    Pushable *pushables = malloc((size_t)dg_room->pushable_count * sizeof(Pushable));
    if (!pushables) { return NO_MEMORY; }

    for (int i = 0; i < dg_room->pushable_count; i++) {
        pushables[i].id       = dg_room->pushables[i].id;
        pushables[i].name     = NULL;
        pushables[i].initial_x = dg_room->pushables[i].x;
        pushables[i].initial_y = dg_room->pushables[i].y;
        pushables[i].x        = dg_room->pushables[i].x;
        pushables[i].y        = dg_room->pushables[i].y;
    }

    r->pushables      = pushables;
    r->pushable_count = dg_room->pushable_count;
    return OK;
}
static Status copy_switches(Room *r, const DG_Room *dg_room) {
    if (dg_room->switch_count <= 0 || dg_room->switches == NULL) { return OK; }

    Switch *switches = malloc((size_t)dg_room->switch_count * sizeof(Switch));
    if (!switches) { return NO_MEMORY; }

    for (int i = 0; i < dg_room->switch_count; i++) {
        switches[i].id        = dg_room->switches[i].id;
        switches[i].x         = dg_room->switches[i].x;
        switches[i].y         = dg_room->switches[i].y;
        switches[i].portal_id = dg_room->switches[i].portal_id;
    }

    r->switches     = switches;
    r->switch_count = dg_room->switch_count;
    return OK;
}
//
static Status process_room(Graph *graph, Room **first_room) {
    DG_Room dg_room = get_next_room();

    Room *r = room_create(dg_room.id, NULL, dg_room.width, dg_room.height);
    if (!r) { return NO_MEMORY; }

    bool *grid_copy = malloc((size_t)dg_room.width * (size_t)dg_room.height * sizeof(bool));
    if (!grid_copy) {
        room_destroy(r);
        return NO_MEMORY;
    }
    memcpy(grid_copy, dg_room.floor_grid,
           (size_t)dg_room.width * (size_t)dg_room.height * sizeof(bool));
    room_set_floor_grid(r, grid_copy);

    if (copy_portals(r, &dg_room) != OK) {
        room_destroy(r);
        return NO_MEMORY;
    }

    if (copy_treasures(r, &dg_room) != OK) {
        room_destroy(r);
        return NO_MEMORY;
    }
    if (copy_switches(r, &dg_room) != OK) {
    room_destroy(r);
    return NO_MEMORY;
}
if (copy_pushables(r, &dg_room) != OK) {
    room_destroy(r);
    return NO_MEMORY;
}
    if (graph_insert(graph, r) != GRAPH_STATUS_OK) {
        room_destroy(r);
        return NO_MEMORY;
    }

    if (*first_room == NULL) { *first_room = r; }
    return OK;
}

Status loader_load_world(const char *config_file,
                         Graph **graph_out,
                         Room **first_room_out,
                         int *num_rooms_out,
                         Charset *charset_out) {

    if (!config_file || !graph_out || !first_room_out || !num_rooms_out || !charset_out) {
        return INVALID_ARGUMENT;
    }

    FILE *f = fopen(config_file, "r");
    if (f == NULL) { return WL_ERR_CONFIG; }
    (void)fclose(f);  /* silence cert-err33-c */

    if (start_datagen(config_file) != 0) { return WL_ERR_DATAGEN; }

    Graph *graph = NULL;
    if (graph_create(room_compare, room_destroy_adapter, &graph) != GRAPH_STATUS_OK) {
        stop_datagen();
        return NO_MEMORY;
    }

    Room *first_room = NULL;
    int room_count = 0;

    while (has_more_rooms()) {
        Status st = process_room(graph, &first_room);
        if (st != OK) {
            stop_datagen();
            graph_destroy(graph);
            return st;
        }
        room_count++;
    }

    copy_charset(dg_get_charset(), charset_out);
    stop_datagen();

    *graph_out      = graph;
    *first_room_out = first_room;
    *num_rooms_out  = room_count;
    return OK;
}