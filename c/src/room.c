#include <stdio.h>
#include "room.h"
#include <stdlib.h>
#include <string.h>


/*
 * Create a new Room with the given identity and dimensions.
 */
Room *room_create(int id, const char *name, int width, int height){
    Room *new_room = malloc(sizeof(Room));
    if (new_room == NULL){
        return NULL;
    }
    
    new_room->id = id;
    new_room->width = (width < 1) ? 1 : width;
    new_room->height = (height < 1) ? 1 : height;
    new_room->portal_count = 0;
    new_room->treasure_count = 0;
    new_room->floor_grid = NULL;
    new_room->portals = NULL;
    new_room->treasures = NULL;
    new_room->name = NULL;  
    new_room->pushables = NULL;      
    new_room->pushable_count = 0;    
    new_room->switches = NULL;       
    new_room->switch_count = 0;      

    if (name != NULL) {
        new_room->name = malloc(strlen(name) + 1);
        if (new_room->name == NULL) {
            free(new_room);
            return NULL;
        }
        strcpy(new_room->name, name);  
    }
    
    return new_room;
}

int room_get_width(const Room *r){
    if(r == NULL){
        return 0;
    }
    return r->width; 
}

int room_get_height(const Room *r){
    if(r == NULL){
        return 0;
    }
    return r->height; 
}

Status room_set_floor_grid(Room *r, bool *floor_grid){
    if(r == NULL){
        return INVALID_ARGUMENT;
    }
    
    if(r->floor_grid != NULL){
        free(r->floor_grid);
    }

    r->floor_grid = floor_grid;
    return OK;
}

Status room_set_portals(Room *r, Portal *portals, int portal_count){
    if(r == NULL){
        return INVALID_ARGUMENT;
    }
    
    if(portal_count > 0 && portals == NULL){
        return INVALID_ARGUMENT;
    }
    
    /* Free OLD portals and their names */
    if(r->portals != NULL){
        for(int i = 0; i < r->portal_count; i++){
            free(r->portals[i].name);
        }
        free(r->portals);
    }
    
    r->portals = portals;
    r->portal_count = portal_count;
    return OK;
}

Status room_set_treasures(Room *r, Treasure *treasures, int treasure_count){
    if(r == NULL){            
        return INVALID_ARGUMENT;
    }
    
    if(treasure_count > 0 && treasures == NULL){
        return INVALID_ARGUMENT;
    }
    
    /* Free OLD treasures and their names */
    if(r->treasures != NULL){
        for(int i = 0; i < r->treasure_count; i++){
            free(r->treasures[i].name);
        }
        free(r->treasures);
    }
    
    r->treasures = treasures;
    r->treasure_count = treasure_count;
    return OK;
}

Status room_place_treasure(Room *r, const Treasure *treasure){
    if(r == NULL || treasure == NULL){            
        return INVALID_ARGUMENT;
    }
    
    /* Calculate new count */
    int new_count = r->treasure_count + 1;
    
    /* Reallocate memory */
    Treasure *new_treasure = realloc(r->treasures, sizeof(Treasure) * new_count);
    if(new_treasure == NULL){
        return NO_MEMORY;
    }
    
    /* Copy the treasure to the LAST position (new_count - 1) */
    new_treasure[new_count - 1] = *treasure;
    
    /* If treasure has a name, duplicate it! */
    if (treasure->name) {
        new_treasure[new_count - 1].name = strdup(treasure->name);
        if (!new_treasure[new_count - 1].name) {
            free(new_treasure);
            return NO_MEMORY;
        }
    }
    
    /* Update room */
    r->treasures = new_treasure;
    r->treasure_count = new_count;
    
    return OK;
}

int room_get_treasure_at(const Room *r, int x, int y){
    if(r == NULL || r->treasures == NULL){
        return -1;
    }
    
    for(int i = 0; i < r->treasure_count; i++){
        if (!r->treasures[i].collected && 
            r->treasures[i].x == x && r->treasures[i].y == y){
            return r->treasures[i].id; 
        }
    }
    return -1;
}

int room_get_portal_destination(const Room *r, int x, int y){
    if(r == NULL || r->portals == NULL){
        return -1;
    }
    
    for(int i = 0; i < r->portal_count; i++){
        if (r->portals[i].x == x && r->portals[i].y == y){
            return r->portals[i].target_room_id; 
        }
    }
    return -1;
}

bool room_is_walkable(const Room *r, int x, int y) {
    if (r == NULL) {
        return false;
    }
    
    /* Check bounds first */
    if (x < 0 || x >= r->width || y < 0 || y >= r->height) {
        return false;  /* Out of bounds */
    }
    
    /* Check pushables first */
    for (int i = 0; i < r->pushable_count; i++) {
        if (r->pushables[i].x == x && r->pushables[i].y == y)
            {return false;}
    }

    /* Handle NULL floor_grid */
    if (r->floor_grid == NULL) {
        /* Perimeter walls with open interior */
        if (x > 0 && x < r->width - 1 && y > 0 && y < r->height - 1) {
            return true;  /* Interior is walkable */
        }
        return false;  /* Perimeter is wall */
    }
    
    /* Normal case: check floor_grid */
    int index = y * r->width + x;
    return r->floor_grid[index];  /* true = floor, false = wall */
}

RoomTileType room_classify_tile(const Room *r, int x, int y, int *out_id) {
    if (r == NULL) {
        return ROOM_TILE_INVALID;
    }

    if (x < 0 || x >= r->width || y < 0 || y >= r->height) {
        return ROOM_TILE_INVALID;
    }

    /* Check treasures */
for (int i = 0; i < r->treasure_count; i++) {
    if (r->treasures[i].collected) { continue; }  // ← add this
    if (r->treasures[i].x != x || r->treasures[i].y != y) { continue; }
    if (out_id != NULL) { *out_id = r->treasures[i].id; }
    return ROOM_TILE_TREASURE;
}

    /* Check portals */
    for (int i = 0; i < r->portal_count; i++) {
        if (r->portals[i].x != x || r->portals[i].y != y) { continue; }
        if (out_id != NULL) { *out_id = r->portals[i].target_room_id; }
        //fprintf(stderr, "portal found: target_room_id=%d\n", r->portals[i].target_room_id);
        return ROOM_TILE_PORTAL;
    }
    

    /* Check pushables */
    for (int i = 0; i < r->pushable_count; i++) {
        if (r->pushables[i].x != x || r->pushables[i].y != y) { continue; }
        if (out_id != NULL) { *out_id = i; }
        return ROOM_TILE_PUSHABLE;
    }

    if (room_is_walkable(r, x, y)) {
        return ROOM_TILE_FLOOR;
    }

    return ROOM_TILE_WALL;
}

//room render//

static char get_base_char(const Room *r, const Charset *charset, int x, int y) {
    char wall = (char)charset->wall;
    char floor_char = (char)charset->floor;
    if (r->floor_grid == NULL) {
        bool is_perimeter = (y == 0 || y == r->height - 1 || x == 0 || x == r->width - 1);
        return (char)(is_perimeter ? wall : floor_char);
    }
    return (char)(r->floor_grid[y * r->width + x] ? floor_char : wall);
}

static char get_entity_char(const Room *r, const Charset *charset, int x, int y) {
    char pushable = (char)charset->pushable;
    char treasure = (char)charset->treasure;
    char portal   = (char)charset->portal;

    if (r->pushables != NULL) {
        for (int i = 0; i < r->pushable_count; i++) {
            if (r->pushables[i].x == x && r->pushables[i].y == y) {
                return pushable;
            }
        }
    }
    if (r->treasures != NULL) {
        for (int i = 0; i < r->treasure_count; i++) {
            if (!r->treasures[i].collected &&
                r->treasures[i].x == x && r->treasures[i].y == y) {
                return treasure;
            }
        }
    }
    if (r->portals != NULL) {
        for (int i = 0; i < r->portal_count; i++) {
            if (r->portals[i].x == x && r->portals[i].y == y) {
                return portal;
            }
        }
    }
    return '\0';
}

Status room_render(const Room *r, const Charset *charset, char *buffer,
                   int buffer_width, int buffer_height) {
    if (r == NULL || charset == NULL || buffer == NULL) {
        return INVALID_ARGUMENT;
    }
    if (buffer_width != r->width || buffer_height != r->height) {
        return INVALID_ARGUMENT;
    }

    for (int y = 0; y < buffer_height; y++) {
        for (int x = 0; x < buffer_width; x++) {
            int index = y * buffer_width + x;

            char entity = get_entity_char(r, charset, x, y);
            char base   = get_base_char(r, charset, x, y);
            buffer[index] = (char)((entity != '\0') ? entity : base);
        }
    }

    return OK;
}

///////////////

Status room_get_start_position(const Room *r, int *x_out, int *y_out){
    if (r == NULL || x_out == NULL || y_out == NULL) {
        return INVALID_ARGUMENT;
    }
    
    if (r->portal_count > 0) {
        *x_out = r->portals[0].x;
        *y_out = r->portals[0].y;
        return OK;
    }
    
    for (int y = 0; y < r->height; y++) {
        for (int x = 0; x < r->width; x++) {
            if (room_is_walkable(r, x, y)) {
                *x_out = x;
                *y_out = y;
                return OK;
            }
        }
    }
    
    return ROOM_NOT_FOUND;
}

void room_destroy(Room *r){
    if (r == NULL) return;
    
    free(r->name);
    free(r->floor_grid);
    
    if (r->portals != NULL) {
        for (int i = 0; i < r->portal_count; i++) {
            free(r->portals[i].name);
        }
        free(r->portals);
    }
    
    if (r->treasures != NULL) {
        for (int i = 0; i < r->treasure_count; i++) {
            free(r->treasures[i].name);
        }
        free(r->treasures);
    }
    free(r->switches);
    free(r->pushables);
    free(r);
}


int room_get_id(const Room *r){
    if(r == NULL){return -1;}
    return r->id;
}

Status room_pick_up_treasure(Room *r, int treasure_id, Treasure **treasure_out){
    if(r == NULL || treasure_out == NULL){return INVALID_ARGUMENT;}
    if(treasure_id < 0){return ROOM_NOT_FOUND;}

    for(int i = 0; i < r->treasure_count; i++){
        if(r->treasures[i].id == treasure_id && !r->treasures[i].collected){
            r->treasures[i].collected = true; 
            *treasure_out = &r->treasures[i];
            return OK;
        }
    }
    return ROOM_NOT_FOUND;
}

void destroy_treasure(Treasure *t){
    free(t);
}

bool room_has_pushable_at(const Room *r, int x, int y, int *pushable_idx_out) {
    if (r == NULL) return false;

    for (int i = 0; i < r->pushable_count; i++) {
        if (r->pushables[i].x == x && r->pushables[i].y == y) {
            if (pushable_idx_out != NULL){
                *pushable_idx_out = i;  }// index, not id
            return true;
        }
    }
    return false;
}

Status room_try_push(Room *r, int pushable_idx, Direction dir) {
    if (r == NULL || pushable_idx < 0 || pushable_idx >= r->pushable_count){
        return INVALID_ARGUMENT;}
    if (dir != DIR_NORTH && dir != DIR_SOUTH && dir != DIR_EAST && dir != DIR_WEST){
        return INVALID_ARGUMENT;}

    int new_x = r->pushables[pushable_idx].x;
    int new_y = r->pushables[pushable_idx].y;

    switch (dir) {
        case DIR_NORTH: new_y -= 1; break;
        case DIR_SOUTH: new_y += 1; break;
        case DIR_EAST:  new_x += 1; break;
        case DIR_WEST:  new_x -= 1; break;
    }

    for (int i = 0; i < r->pushable_count; i++) {
    if (i == pushable_idx) continue;
    if (r->pushables[i].x == new_x && r->pushables[i].y == new_y){
        return ROOM_IMPASSABLE;}
}

RoomTileType tile = room_classify_tile(r, new_x, new_y, NULL);


if (tile == ROOM_TILE_INVALID || tile == ROOM_TILE_WALL||
    tile == ROOM_TILE_TREASURE || tile == ROOM_TILE_PORTAL){
    return ROOM_IMPASSABLE;
}

    r->pushables[pushable_idx].x = new_x;
    r->pushables[pushable_idx].y = new_y;
    return OK;
}