import ctypes
from ..bindings import lib, Status, Direction, GameEngine as CGameEngine
from .player import Player
from .exceptions import status_to_exception


class GameEngine:
    def __init__(self, config_path: str):
        self._eng = CGameEngine(None)
        status = Status(lib.game_engine_create(
            config_path.encode("utf-8"),
            ctypes.byref(self._eng)
        ))
        if status != Status.OK:
            raise status_to_exception(status, f"game_engine_create failed: {config_path}")

        player_ptr = lib.game_engine_get_player(self._eng)
        if player_ptr is None:
            raise RuntimeError("game_engine_get_player returned None")
        self._player = Player(player_ptr)

    @property
    def player(self) -> Player:
        return self._player

    def destroy(self) -> None:
        if self._eng:
            lib.game_engine_destroy(self._eng)
            self._eng = None

    def move_player(self, direction: Direction) -> None:
        status = Status(lib.game_engine_move_player(self._eng, int(direction)))
        if status != Status.OK:
            raise status_to_exception(status, f"move_player failed: {direction}")

    def render_current_room(self) -> str:
        out = ctypes.c_char_p()
        status = Status(lib.game_engine_render_current_room(
            self._eng, ctypes.byref(out)
        ))
        if status != Status.OK:
            raise status_to_exception(status, "render_current_room failed")
        result = out.value.decode("utf-8")
        lib.game_engine_free_string(out)
        return result

    def get_room_count(self) -> int:
        count = ctypes.c_int(0)
        status = Status(lib.game_engine_get_room_count(self._eng, ctypes.byref(count)))
        if status != Status.OK:
            raise status_to_exception(status, "get_room_count failed")
        return count.value

    def get_room_dimensions(self) -> tuple[int, int]:
        width = ctypes.c_int(0)
        height = ctypes.c_int(0)
        status = Status(lib.game_engine_get_room_dimensions(
            self._eng, ctypes.byref(width), ctypes.byref(height)
        ))
        if status != Status.OK:
            raise status_to_exception(status, "get_room_dimensions failed")
        return (width.value, height.value)

    def get_room_ids(self) -> list[int]:
        arr = ctypes.POINTER(ctypes.c_int)()
        count = ctypes.c_int(0)
        status = Status(lib.game_engine_get_room_ids(
            self._eng, ctypes.byref(arr), ctypes.byref(count)
        ))
        if status != Status.OK:
            raise status_to_exception(status, "get_room_ids failed")

        ids = [arr[i] for i in range(count.value)]
        lib.game_engine_free_string(arr)
        return ids

    def reset(self) -> None:
        status = Status(lib.game_engine_reset(self._eng))
        if status != Status.OK:
            raise status_to_exception(status, "reset failed")
        
    
    def get_total_treasures(self) -> int:
        return lib.game_engine_get_total_treasures()
        
    def total_treasures(self) -> bool:
        """Return True if all treasures in dg_room are collected, else False."""
        return bool(lib.game_engine_total_treasures(self._eng))