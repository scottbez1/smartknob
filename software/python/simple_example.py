import os
import sys
if __name__ == '__main__':
    if 'PIPENV_ACTIVE' not in os.environ:
        sys.exit(f'This script should be run in a Pipenv.\n\nRun it as:\npipenv run python {os.path.basename(__file__)}')

# Place imports below this line
import logging
import math

from smartknob_io import (
    ask_for_serial_port,
    smartknob_context
)
from proto_gen import smartknob_pb2

def _run_example():
    logging.basicConfig(level=logging.INFO)

    p = ask_for_serial_port()
    with smartknob_context(p) as s:
        # Initialize with an empty state object
        last_state = smartknob_pb2.SmartKnobState()

        # Callback function to handle state updates
        def log_state(new_state):
            nonlocal last_state

            # We'll log the state if it's changed substantially since the last state we recieved
            config_changed = last_state.config.SerializeToString(deterministic=True) != new_state.config.SerializeToString(deterministic=True)
            position_changed = last_state.current_position != new_state.current_position
            sub_position_large_change = abs(last_state.sub_position_unit * last_state.config.position_width_radians - new_state.sub_position_unit * new_state.config.position_width_radians) > math.radians(5)
            press_nonce_changed = last_state.press_nonce != new_state.press_nonce
            if config_changed or position_changed or sub_position_large_change or press_nonce_changed:
                logging.info('State: ' + str(new_state))
                last_state = new_state

        # Register our state handler function
        s.add_handler('smartknob_state', log_state)

        # Run forever, set config when enter is pressed
        while True:
            input()
            config = smartknob_pb2.SmartKnobConfig()
            config.position = 0
            config.min_position = 0
            config.max_position = 5
            config.position_width_radians = math.radians(10)
            config.detent_strength_unit = 1
            config.endstop_strength_unit = 1
            config.snap_point = 1.1
            config.text = "From Python!"
            s.set_config(config)

if __name__ == '__main__':
    _run_example()
