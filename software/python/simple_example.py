import os
import sys
if __name__ == '__main__':
    if 'PIPENV_ACTIVE' not in os.environ:
        sys.exit(f'This script should be run in a Pipenv.\n\nRun it as:\npipenv run python {os.path.basename(__file__)}')

# Place imports below this line
import logging
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

        # Keep running until enter is pressed
        input()

if __name__ == '__main__':
    _run_example()
