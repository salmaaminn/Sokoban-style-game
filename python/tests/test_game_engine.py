import unittest
import os
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.models.exceptions import GameEngineError, InvalidArgumentError, ImpassableError
from treasure_runner.bindings import Direction

CONFIG_PATH = CONFIG_PATH = "../assets/starter.ini"


class TestGameEngine(unittest.TestCase):

    def setUp(self):
        self.engine = GameEngine(CONFIG_PATH)

    def tearDown(self):
        self.engine.destroy()

    def test_create_valid(self):
        self.assertIsNotNone(self.engine)

    def test_create_invalid_path(self):
        with self.assertRaises(GameEngineError):
            GameEngine("/nonexistent/path.ini")

    def test_player_not_none(self):
        self.assertIsNotNone(self.engine.player)

    def test_player_initial_position(self):
        x, y = self.engine.player.get_position()
        self.assertGreaterEqual(x, 0)
        self.assertGreaterEqual(y, 0)

    def test_player_initial_room(self):
        room = self.engine.player.get_room()
        self.assertGreaterEqual(room, 0)

    def test_get_room_count(self):
        count = self.engine.get_room_count()
        self.assertGreater(count, 0)

    def test_get_room_dimensions(self):
        w, h = self.engine.get_room_dimensions()
        self.assertGreater(w, 0)
        self.assertGreater(h, 0)


    def test_move_player_all_directions(self):
        for direction in [Direction.NORTH, Direction.SOUTH, Direction.EAST, Direction.WEST]:
            self.engine.reset()
            try:
                self.engine.move_player(direction)
            except ImpassableError:
                pass  # blocked is acceptable

    def test_move_and_reset(self):
        x_before, y_before = self.engine.player.get_position()
        room_before = self.engine.player.get_room()
        for direction in [Direction.SOUTH, Direction.WEST, Direction.NORTH, Direction.EAST]:
            try:
                self.engine.move_player(direction)
                break
            except ImpassableError:
                continue
        self.engine.reset()
        x_after, y_after = self.engine.player.get_position()
        room_after = self.engine.player.get_room()
        self.assertEqual(x_before, x_after)
        self.assertEqual(y_before, y_after)
        self.assertEqual(room_before, room_after)

    def test_render_current_room(self):
        result = self.engine.render_current_room()
        self.assertIsInstance(result, str)
        self.assertGreater(len(result), 0)

    def test_create_multiple_instances(self):
        eng2 = GameEngine(CONFIG_PATH)
        self.assertIsNotNone(eng2)
        eng2.destroy()

    def test_collected_count_initial(self):
        self.assertEqual(self.engine.player.get_collected_count(), 0)

    def test_reset_clears_collected(self):
        self.engine.reset()
        self.assertEqual(self.engine.player.get_collected_count(), 0)


if __name__ == "__main__":
    unittest.main()