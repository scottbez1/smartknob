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
        last_state = smartknob_pb2.SmartKnobState()
        def log_state(message):
            nonlocal last_state
            if last_state.config.SerializeToString(deterministic=True) != message.config.SerializeToString(deterministic=True):
                logging.info('State: ' + str(message))
                last_state = message
        s.add_handler('smartknob_state', log_state)
        s.request_state()

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
