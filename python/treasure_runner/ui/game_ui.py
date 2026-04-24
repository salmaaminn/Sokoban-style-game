import curses
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.bindings import Direction
from treasure_runner.models.exceptions import ImpassableError

class GameUI:
    """Handles display and input for the dungeon game."""

    def __init__(self, stdscr, engine):
        self.stdscr = stdscr
        self.engine = engine
        self.status_msg = ""

        if curses.has_colors():
            curses.start_color()
            curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_WHITE)
            curses.init_pair(2, curses.COLOR_WHITE, curses.COLOR_BLUE)
        self.stdscr.bkgd(' ', curses.color_pair(2))

    def draw(self):
        """Draw the current room and status bar."""
        self.stdscr.clear()
        self._draw_board()
        self._draw_status_bar()
        self.stdscr.refresh()

    def _draw_board(self):
        """Fetch the current room string from the engine and render it."""
        height, width = self.stdscr.getmaxyx()
        room_string = self.engine.render_current_room()  # calls game_engine_render_current_room
        lines = room_string.split('\n')

        for y, line in enumerate(lines):
            if y >= height - 1:  # leave bottom row for status bar
                break
            try:
                self.stdscr.addstr(y, 0, line[:width - 1])
            except curses.error:
                pass

    def _draw_status_bar(self):
        """Draw the current status message at the bottom."""
        height, width = self.stdscr.getmaxyx()
        msg = self.status_msg[:width - 1]
        self.stdscr.attron(curses.A_REVERSE)
        self.stdscr.addstr(height - 1, 0, msg.ljust(width - 1))
        self.stdscr.attroff(curses.A_REVERSE)

    def handle_input(self, key) -> bool:
        if key == ord('q'):
            return False

        direction_map = {
            curses.KEY_UP:    Direction.NORTH,
            curses.KEY_DOWN:  Direction.SOUTH,
            curses.KEY_RIGHT: Direction.EAST,
            curses.KEY_LEFT:  Direction.WEST,
        }

        if key in direction_map:
            try:
                self.engine.move_player(direction_map[key])
                self.status_msg = ""
            except ImpassableError:
                self.status_msg = "Blocked!"
            except Exception as error:
                self.status_msg = f"Error: {error}"

        return True
