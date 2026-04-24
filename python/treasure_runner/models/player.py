import ctypes
from ..bindings import lib, Treasure


class Player:
    def __init__(self, ptr):
        self._ptr = ptr

    def get_room(self) -> int:
        return lib.player_get_room(self._ptr)

    def get_position(self) -> tuple[int, int]:
        x = ctypes.c_int(0)
        y = ctypes.c_int(0)
        lib.player_get_position(self._ptr, ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def get_collected_count(self) -> int:
        return lib.player_get_collected_count(self._ptr)

    def has_collected_treasure(self, treasure_id: int) -> bool:
        return bool(lib.player_has_collected_treasure(self._ptr, ctypes.c_int(treasure_id)))

    def get_collected_treasures(self) -> list[dict]:
        count = ctypes.c_int(0)
        arr = lib.player_get_collected_treasures(self._ptr, ctypes.byref(count))
        result = []
        if not arr:
            return result
        for i in range(count.value):
            treasure = arr[i]
            result.append({
                "id":               treasure.id,
                "name":             treasure.name.decode("utf-8") if treasure.name else None,
                "starting_room_id": treasure.starting_room_id,
                "initial_x":        treasure.initial_x,
                "initial_y":        treasure.initial_y,
                "x":                treasure.x,
                "y":                treasure.y,
                "collected":        treasure.collected,
            })
        return result
    