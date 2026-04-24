#!/usr/bin/env python3
"""Deterministic system integration test runner for Treasure Runner."""

import os
import argparse
from treasure_runner.bindings import Direction
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.models.exceptions import GameError, ImpassableError


def player_state_str(engine: GameEngine) -> str:
    player = engine.player
    x, y = player.get_position()
    room = player.get_room()
    collected = player.get_collected_count()
    return f"room={room}|x={x}|y={y}|collected={collected}"

#find first tile
def find_entry_direction(engine: GameEngine) -> Direction:
    """Try SOUTH, WEST, NORTH, EAST and return first valid entry direction."""
    order = [Direction.SOUTH, Direction.WEST, Direction.NORTH, Direction.EAST]

    for direction in order:
        engine.reset() #each direction test should start from spawn, so reset before each try
        before_room = engine.player.get_room()
        before_pos = engine.player.get_position()

        try:
            engine.move_player(direction)
        except ImpassableError:
            continue
        except GameError:
            engine.reset()
            continue

        after_room = engine.player.get_room()
        after_pos = engine.player.get_position()

        if after_room == before_room and after_pos != before_pos:
            engine.reset() 
            return direction #if found valid direction return and reset engine

        engine.reset()

    raise RuntimeError("No valid entry direction found")


def run_sweep(engine: GameEngine, direction: Direction, step: int, log_lines: list) -> int:
    
    dir_name = direction.name
    
    #log the start of this sweep phase
    log_lines.append(f"SWEEP_START|phase=SWEEP_{dir_name}|dir={dir_name}")

    #only tracks states within this sweep
    seen_states = set()

    stop_reason = "BLOCKED"
    
    #successful moves
    moves = 0

    while True:
        # player state before attempting the move
        before_state = player_state_str(engine)
        before_collected = engine.player.get_collected_count()

        #cycle detection
        if before_state in seen_states:
            stop_reason = "CYCLE_DETECTED"
            break
        
        #not seen before 
        seen_states.add(before_state) #record this state so we can detect if we return to it

        # Increment step (blocked moves also get step)
        step += 1

        try:
            #move player
            engine.move_player(direction)
            
        except ImpassableError:
            # C returned ROOM_IMPASSABLE
            #after state for log 
            after_state = player_state_str(engine)
            after_collected = engine.player.get_collected_count()
            delta = after_collected - before_collected  # will be 0 since move was blocked
            
            # Log the blocked attempt
            log_lines.append(
                f"MOVE|step={step}|phase=SWEEP_{dir_name}|dir={dir_name}"
                f"|result=BLOCKED|before={before_state}|after={after_state}"
                f"|delta_collected={delta}"
            )
            stop_reason = "BLOCKED"
            break  #can't go further in this direction, break sweep
            
        except GameError:
            # C returned non-impassable error
            # result=ERROR
            after_state = player_state_str(engine)
            after_collected = engine.player.get_collected_count()
            delta = after_collected - before_collected
            log_lines.append(
                f"MOVE|step={step}|phase=SWEEP_{dir_name}|dir={dir_name}"
                f"|result=ERROR|before={before_state}|after={after_state}"
                f"|delta_collected={delta}"
            )
            stop_reason = "BLOCKED" 
            break

        # Move did not raise an exception 
        after_state = player_state_str(engine)
        after_collected = engine.player.get_collected_count()
        delta = after_collected - before_collected  # > 0 if treasure was collected this move

        # Silent stuck check — C returned OK but nothing actually changed
        # don't log this attempt and don't count the step
        if before_state == after_state:
            step -= 1  # undo the increment — this attempt doesn't count
            stop_reason = "BLOCKED"
            break

        #successful move, increment move counter
        moves += 1
        
        # Log the successful move with full before/after state and treasure delta
        log_lines.append(
            f"MOVE|step={step}|phase=SWEEP_{dir_name}|dir={dir_name}"
            f"|result=OK|before={before_state}|after={after_state}"
            f"|delta_collected={delta}"
        )
        # Loop continues try to move another step in the same direction

    # Sweep is done log the summary with why it stopped and how many tiles were traversed
    log_lines.append(f"SWEEP_END|phase=SWEEP_{dir_name}|reason={stop_reason}|moves={moves}")
    
    # Return updated step so the next sweep continues counting from here
    return step

def run_integration(config_path: str, log_path: str) -> None:

    log_lines = []

    # First line of every log
    log_lines.append(f"RUN_START|config={config_path}")

    #allocates game world here
    engine = GameEngine(config_path)

    # where the player spawned always step=0, before any moves
    spawn_state = player_state_str(engine)
    log_lines.append(f"STATE|step=0|phase=SPAWN|state={spawn_state}")

    # Probe SOUTH, WEST, NORTH, EAST in order to find a direction that
    # steps the player off the portal onto a floor tile in the same room
    # Internally resets the engine between each probe attempt
    entry_dir = find_entry_direction(engine)
    
    # Log which direction was selected, no state yet
    log_lines.append(f"ENTRY|direction={entry_dir.name}")

    # Reset to clean initial state before executing the real entry move
    engine.reset()
    
    #  before the entry move for the log
    before_state = player_state_str(engine)
    before_collected = engine.player.get_collected_count()
    
    # Step counter starts at 1 — step 0 was spawn, no move happened there
    step = 1

    try:
        # Execute the actual entry move
        engine.move_player(entry_dir)
        
        # Move succeeded, log the result with before/after state and treasure delta
        after_state = player_state_str(engine)
        after_collected = engine.player.get_collected_count()
        delta = after_collected - before_collected  # 0 unless treasure was on that tile
        
        log_lines.append(
            f"MOVE|step={step}|phase=ENTRY|dir={entry_dir.name}"
            f"|result=OK|before={before_state}|after={after_state}"
            f"|delta_collected={delta}"
        )
        
    except GameError:
        # Entry move failed, the run cannot continue
        after_state = player_state_str(engine)
        after_collected = engine.player.get_collected_count()
        delta = after_collected - before_collected
        
        # Log the failed entry move with result=ERROR
        log_lines.append(
            f"MOVE|step={step}|phase=ENTRY|dir={entry_dir.name}"
            f"|result=ERROR|before={before_state}|after={after_state}"
            f"|delta_collected={delta}"
        )
        
        #  error case
        log_lines.append("TERMINATED: Initial Move Error")

        # Write RUN_END with whatever was collected (likely 0)
        collected = engine.player.get_collected_count()
        log_lines.append(f"RUN_END|steps={step}|collected_total={collected}")

        # Write log to disk even on failure — autograder still reads it
        with open(log_path, "w") as f:
            f.write("\n".join(log_lines) + "\n")
        
        # Tell C to free all allocated memory before exiting
        engine.destroy()
        return 

    # when move succeeded run the four directional sweeps in fixed order
    # Each sweep moves as far as possible in that direction
    # step is passed in and returned so the counter is continuous across all sweeps
    sweeps = [Direction.SOUTH, Direction.WEST, Direction.NORTH, Direction.EAST]
    for sweep_dir in sweeps:
        step = run_sweep(engine, sweep_dir, step, log_lines)

    # final player state after all sweeps are done
    final_state = player_state_str(engine)
    log_lines.append(f"STATE|step={step}|phase=FINAL|state={final_state}")

    # total steps across entire run and total treasures collected
    collected = engine.player.get_collected_count()
    log_lines.append(f"RUN_END|steps={step}|collected_total={collected}")

    # Write the entire log
    with open(log_path, "w") as f:
        f.write("\n".join(log_lines) + "\n")

    # Tell C to free all allocated memory (mem will leak otherwise)z
    engine.destroy()


def parse_args():
    parser = argparse.ArgumentParser(description="Treasure Runner integration test logger")
    parser.add_argument("--config", required=True, help="Path to generator config file")
    parser.add_argument("--log", required=True, help="Output log path")
    return parser.parse_args()


def main():
    args = parse_args()
    config_path = os.path.abspath(args.config)
    log_path = os.path.abspath(args.log)
    run_integration(config_path, log_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())