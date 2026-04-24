import argparse
import curses
import os
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.ui.game_ui import GameUI

def main(stdscr, config_path):
    engine = GameEngine(config_path)
    ui = GameUI(stdscr, engine)
    running = True
    while running:
        ui.draw()
        key = stdscr.getch()
        # ui.handle_input will set running = False when 'q' is pressed
        running = ui.handle_input(key)
        #if (call total treasure collected in game engine) == 1:
        #    ui handle that, which will draw the winning screen and exit
        collected = engine.player.get_collected_count()
        total = engine.get_total_treasures()
        ui.status_msg = f"{collected}/{total} treasures collected"
        if collected == total:
            ui.status_msg = "Congratulations! You collected all treasures!"
            ui.draw()
            curses.napms(3000)  # pause for 3 seconds to show the message
            break

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", required=True)
    args = parser.parse_args()
    config_path = os.path.abspath(args.config)
    curses.wrapper(main, config_path)  # passes config_path as second arg to main