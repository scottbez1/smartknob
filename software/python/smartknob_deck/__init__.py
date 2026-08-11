"""
Turn a SmartKnob into a "deck" for controlling the computer it's plugged into.

Press the knob to cycle between controls (system volume, display brightness);
rotate to adjust the active one. All of the logic lives on the host: the
firmware just gets a SmartKnobConfig for the active control and reports
rotation/presses back.

Run it with:
    pipenv run python -m smartknob_deck
"""
