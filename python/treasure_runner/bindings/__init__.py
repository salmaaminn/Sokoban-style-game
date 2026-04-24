import ctypes
from .bindings import lib, Status, Direction, Treasure, GameEngine, Player, Room, GameEngineStatus
# game engine

lib.game_engine_create.argtypes = [ctypes.c_char_p, ctypes.POINTER(GameEngine)]
lib.game_engine_create.restype  = ctypes.c_int

lib.game_engine_destroy.argtypes = [GameEngine]
lib.game_engine_destroy.restype  = None

lib.game_engine_get_player.argtypes = [GameEngine]
lib.game_engine_get_player.restype  = Player

lib.game_engine_move_player.argtypes = [GameEngine, ctypes.c_int]
lib.game_engine_move_player.restype  = ctypes.c_int

lib.game_engine_render_current_room.argtypes = [GameEngine, ctypes.POINTER(ctypes.c_char_p)]
lib.game_engine_render_current_room.restype  = ctypes.c_int

lib.game_engine_get_room_count.argtypes = [GameEngine, ctypes.POINTER(ctypes.c_int)]
lib.game_engine_get_room_count.restype  = ctypes.c_int

lib.game_engine_get_room_dimensions.argtypes = [
    GameEngine,
    ctypes.POINTER(ctypes.c_int),
    ctypes.POINTER(ctypes.c_int),
]
lib.game_engine_get_room_dimensions.restype = ctypes.c_int

lib.game_engine_get_room_ids.argtypes = [
    GameEngine,
    ctypes.POINTER(ctypes.POINTER(ctypes.c_int)),
    ctypes.POINTER(ctypes.c_int),
]
lib.game_engine_get_room_ids.restype = ctypes.c_int

lib.game_engine_get_room_ids.argtypes = [GameEngine, ctypes.c_int, ctypes.POINTER(Room)]
lib.game_engine_get_room_ids.restype  = ctypes.c_int

lib.game_engine_reset.argtypes = [GameEngine]
lib.game_engine_reset.restype  = ctypes.c_int

# total_treasures returns an int (nonzero = all collected, 0 = not all collected)
lib.game_engine_total_treasures.argtypes = [GameEngine]  # dg_room pointer
lib.game_engine_total_treasures.restype = ctypes.c_int

lib.game_engine_get_total_treasures.argtypes = []  # dg_room pointer
lib.game_engine_get_total_treasures.restype = ctypes.c_int
# player operations

# player_get_room returns int directly
lib.player_get_room.argtypes = [Player]
lib.player_get_room.restype  = ctypes.c_int

# player_get_collected_count returns int directly
lib.player_get_collected_count.argtypes = [Player]
lib.player_get_collected_count.restype  = ctypes.c_int

# player_has_collected_treasure returns bool directly
lib.player_has_collected_treasure.argtypes = [Player, ctypes.c_int]
lib.player_has_collected_treasure.restype  = ctypes.c_bool

# player_get_collected_treasures returns pointer directly, takes count_out
lib.player_get_collected_treasures.argtypes = [Player, ctypes.POINTER(ctypes.c_int)]
lib.player_get_collected_treasures.restype  = ctypes.POINTER(Treasure)

# player_reset_to_start takes room_id, x, y
lib.player_reset_to_start.argtypes = [Player, ctypes.c_int, ctypes.c_int, ctypes.c_int]
lib.player_reset_to_start.restype  = ctypes.c_int

# memory

lib.game_engine_free_string.argtypes = [ctypes.c_void_p]
lib.game_engine_free_string.restype  = None

lib.destroy_treasure.argtypes = [ctypes.POINTER(Treasure)]
lib.destroy_treasure.restype  = None
