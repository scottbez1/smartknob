"""
Deck logic: which control is active, and how knob positions map to OS values.

This module is deliberately free of serial/protobuf/OS dependencies so it can be
unit tested on its own. The caller supplies a `sender` callable that turns a
KnobConfig into a SmartKnobConfig message and puts it on the wire, and feeds
KnobState snapshots (built from incoming SmartKnobState messages) into
handle_state().
"""

from dataclasses import dataclass
import logging
import time

from .controls import ControlUnavailable

logger = logging.getLogger(__name__)

# Don't spam configs when the knob looks out of sync (e.g. while it reboots)
RESEND_INTERVAL_SECONDS = 1.0

# How long after the last rotation before we trust the OS's value again. Without
# this, a slow volume/brightness write could be read back mid-turn and fight the
# user for control of the position.
IDLE_BEFORE_POLL_SECONDS = 0.75


@dataclass(frozen=True)
class KnobConfig:
    """Host-agnostic view of the fields of PB.SmartKnobConfig that we set."""
    position: int
    position_nonce: int
    min_position: int
    max_position: int
    position_width_radians: float
    detent_strength_unit: float
    endstop_strength_unit: float
    snap_point: float
    text: str
    led_hue: int


@dataclass(frozen=True)
class KnobState:
    """The parts of PB.SmartKnobState that the deck cares about."""
    current_position: int
    press_nonce: int
    config_text: str


class DeckController(object):
    def __init__(self, controls, sender, poll_interval=2.0, clock=time.monotonic):
        if not controls:
            raise ValueError('need at least one control')
        self._controls = list(controls)
        self._sender = sender
        self._poll_interval = poll_interval
        self._clock = clock

        self._index = 0
        self._nonce = 0
        self._value = None
        self._pending_value = None
        self._press_nonce = None
        self._last_write = 0.0
        self._last_poll = 0.0
        self._last_rotation = 0.0
        self._last_config_sent = 0.0

    @property
    def active_control(self):
        return self._controls[self._index]

    @property
    def value(self):
        """Value the deck believes the active control is currently at."""
        return self._value

    def start(self):
        """Send the initial config for the first control."""
        self._activate()

    def handle_state(self, state):
        """Process a state snapshot from the knob."""
        if self._handle_press(state.press_nonce):
            # The config just changed; this snapshot's position belongs to the
            # control we were on a moment ago, so don't apply it.
            return

        if state.config_text != self.active_control.label:
            # The knob isn't running our config (it just booted, or our config
            # was lost) - re-send it rather than acting on unrelated positions.
            if self._clock() - self._last_config_sent >= RESEND_INTERVAL_SECONDS:
                logger.info('Knob is showing "%s"; re-sending %s config',
                            state.config_text, self.active_control.label)
                self._send()
            return

        value = self.active_control.value_for_position(state.current_position)
        if value != self._value:
            self._value = value
            self._pending_value = value
            self._last_rotation = self._clock()
            self._flush_pending()

    def tick(self):
        """Periodic work: flush coalesced writes, follow external changes."""
        self._flush_pending()
        self._poll_external()

    def next_control(self):
        """Switch to the next control in the rotation."""
        self._index = (self._index + 1) % len(self._controls)
        self._activate()

    def _handle_press(self, press_nonce):
        if self._press_nonce is None:
            # First state we've seen; establish a baseline rather than treating
            # whatever the knob's counter happens to be at as a fresh press.
            self._press_nonce = press_nonce
            return False
        if press_nonce == self._press_nonce:
            return False

        self._press_nonce = press_nonce
        if len(self._controls) == 1:
            return False
        self.next_control()
        return True

    def _activate(self):
        control = self.active_control
        self._pending_value = None
        try:
            self._value = control.read()
        except ControlUnavailable as e:
            logger.warning('Could not read %s: %s', control.label, e)
            # Keep whatever we last knew, snapped into this control's range
            self._value = control.clamp(
                control.min_value if self._value is None else self._value)
        logger.info('%s: %d', control.label, self._value)
        self._send()

    def _send(self):
        control = self.active_control
        self._nonce = (self._nonce + 1) % 256
        self._sender(KnobConfig(
            position=control.position_for_value(self._value),
            position_nonce=self._nonce,
            min_position=control.min_value,
            max_position=control.max_value,
            position_width_radians=control.position_width_radians,
            detent_strength_unit=control.detent_strength_unit,
            endstop_strength_unit=control.endstop_strength_unit,
            snap_point=control.snap_point,
            text=control.label,
            led_hue=control.led_hue,
        ))
        self._last_config_sent = self._clock()

    def _flush_pending(self):
        if self._pending_value is None:
            return
        control = self.active_control
        now = self._clock()
        if now - self._last_write < control.min_write_interval:
            return

        value = self._pending_value
        self._pending_value = None
        self._last_write = now
        try:
            control.write(value)
        except ControlUnavailable as e:
            logger.warning('Could not set %s: %s', control.label, e)
        else:
            logger.debug('%s -> %d', control.label, value)

    def _poll_external(self):
        """Follow changes made elsewhere (volume keys, another app, etc.)."""
        if self._pending_value is not None:
            return
        now = self._clock()
        if now - self._last_rotation < IDLE_BEFORE_POLL_SECONDS:
            return
        if now - self._last_poll < self._poll_interval:
            return
        self._last_poll = now

        control = self.active_control
        try:
            value = control.read()
        except ControlUnavailable as e:
            logger.debug('Could not read %s: %s', control.label, e)
            return
        if value != self._value:
            logger.info('%s changed externally: %d -> %d',
                        control.label, self._value, value)
            self._value = value
            self._send()
