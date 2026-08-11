"""
Tests for the SmartKnob deck logic.

These only exercise the host-side logic, so they need no hardware and no
third-party packages:
    python -m unittest test_smartknob_deck
"""

import logging
import math
import unittest

from smartknob_deck.controller import (
    IDLE_BEFORE_POLL_SECONDS,
    RESEND_INTERVAL_SECONDS,
    DeckController,
    KnobState,
)
from smartknob_deck.controls import (
    ALL_CONTROLS,
    Control,
    ControlUnavailable,
    build_controls,
)


class FakeClock(object):
    def __init__(self):
        self.now = 1000.0

    def __call__(self):
        return self.now

    def advance(self, seconds):
        self.now += seconds


class FullRangeControl(Control):
    """Bare Control, for checking the travel maths the real controls inherit."""


class FakeControl(Control):
    # Pinned so controller tests don't depend on the travel budget
    position_width_radians = math.radians(5)

    def __init__(self, key='fake', label='Fake', value=0, min_value=0,
                 max_value=100, led_hue=10, min_write_interval=0):
        self.key = key
        self.label = label
        self.min_value = min_value
        self.max_value = max_value
        self.led_hue = led_hue
        self.min_write_interval = min_write_interval
        self.value = value
        self.writes = []
        self.read_error = None
        self.write_error = None

    def _read(self):
        if self.read_error is not None:
            raise self.read_error
        return self.value

    def _write(self, value):
        if self.write_error is not None:
            raise self.write_error
        self.value = value
        self.writes.append(value)


def make_controller(controls, clock=None, poll_interval=2.0):
    sent = []
    controller = DeckController(
        controls,
        sent.append,
        poll_interval=poll_interval,
        clock=clock or FakeClock(),
    )
    return controller, sent


def state(position, press_nonce=0, config_text='Fake'):
    return KnobState(current_position=position, press_nonce=press_nonce,
                     config_text=config_text)


class ControlMappingTest(unittest.TestCase):
    def test_position_is_the_value(self):
        control = FakeControl(min_value=5, max_value=100)
        self.assertEqual(control.value_for_position(5), 5)
        self.assertEqual(control.value_for_position(37), 37)
        self.assertEqual(control.position_for_value(100), 100)

    def test_values_are_clamped_to_the_range(self):
        control = FakeControl(min_value=5, max_value=100)
        self.assertEqual(control.value_for_position(-3), 5)
        self.assertEqual(control.value_for_position(140), 100)
        self.assertEqual(control.position_for_value(-10), 5)
        self.assertEqual(control.position_for_value(500), 100)

    def test_read_returns_an_int_in_range(self):
        control = FakeControl(min_value=5, max_value=100, value=42.6)
        self.assertEqual(control.read(), 43)

    def test_write_clamps_before_touching_the_os(self):
        control = FakeControl(min_value=0, max_value=100)
        control.write(240)
        control.write(-8)
        self.assertEqual(control.writes, [100, 0])

    def test_build_controls_rejects_unknown_keys(self):
        with self.assertRaises(ControlUnavailable):
            build_controls(['nope'])


class TravelTest(unittest.TestCase):
    """Every control sweeps the same 300 degrees, centred by the firmware."""

    def travel_degrees(self, control):
        span = control.max_value - control.min_value
        return span * math.degrees(control.position_width_radians)

    def test_every_real_control_travels_300_degrees(self):
        for key, factory in sorted(ALL_CONTROLS.items()):
            with self.subTest(control=key):
                # Constructed directly: no OS access happens until read/write
                self.assertAlmostEqual(self.travel_degrees(factory()), 300,
                                       places=6)

    def test_detent_width_follows_the_range(self):
        # 100 steps over 300 degrees is 3 degrees each...
        self.assertAlmostEqual(
            math.degrees(FullRangeControl().position_width_radians), 3.0)
        # ...and a narrower range spreads the same travel over fewer steps
        narrow = FullRangeControl()
        narrow.min_value, narrow.max_value = 0, 10
        self.assertAlmostEqual(
            math.degrees(narrow.position_width_radians), 30.0)
        self.assertAlmostEqual(self.travel_degrees(narrow), 300)

    def test_single_value_range_does_not_divide_by_zero(self):
        control = FullRangeControl()
        control.min_value = control.max_value = 50
        self.assertAlmostEqual(
            math.degrees(control.position_width_radians), 300)


class ActivationTest(unittest.TestCase):
    def test_start_sends_config_for_the_current_os_value(self):
        control = FakeControl(value=30, led_hue=96)
        controller, sent = make_controller([control])

        controller.start()

        self.assertEqual(len(sent), 1)
        config = sent[0]
        self.assertEqual(config.position, 30)
        self.assertEqual(config.min_position, 0)
        self.assertEqual(config.max_position, 100)
        self.assertEqual(config.text, 'Fake')
        self.assertEqual(config.led_hue, 96)
        self.assertEqual(controller.value, 30)

    def test_unreadable_control_falls_back_to_its_minimum(self):
        control = FakeControl(min_value=5, max_value=100)
        control.read_error = ControlUnavailable('no display')
        controller, sent = make_controller([control])

        with self.assertLogs('smartknob_deck.controller', level='WARNING'):
            controller.start()

        self.assertEqual(controller.value, 5)
        self.assertEqual(sent[0].position, 5)
        self.assertEqual(sent[0].min_position, 5)

    def test_position_nonce_stays_within_a_byte(self):
        # The firmware stores position_nonce in a uint8
        controls = [FakeControl(label='A'), FakeControl(label='B')]
        controller, sent = make_controller(controls)
        controller.start()
        for _ in range(300):
            controller.next_control()

        self.assertEqual(len(sent), 301)
        self.assertTrue(all(0 <= config.position_nonce < 256 for config in sent))


class RotationTest(unittest.TestCase):
    def test_rotation_writes_the_new_value(self):
        control = FakeControl(value=30)
        controller, _ = make_controller([control])
        controller.start()

        controller.handle_state(state(20))

        self.assertEqual(control.writes, [20])
        self.assertEqual(controller.value, 20)

    def test_repeated_position_is_not_rewritten(self):
        control = FakeControl(value=30)
        controller, _ = make_controller([control])
        controller.start()

        controller.handle_state(state(20))
        controller.handle_state(state(20))

        self.assertEqual(control.writes, [20])

    def test_writes_are_coalesced_while_rate_limited(self):
        clock = FakeClock()
        control = FakeControl(value=0, min_write_interval=0.15)
        controller, _ = make_controller([control], clock=clock)
        controller.start()

        controller.handle_state(state(1))
        controller.handle_state(state(2))
        controller.handle_state(state(3))
        self.assertEqual(control.writes, [1])

        clock.advance(0.2)
        controller.tick()
        self.assertEqual(control.writes, [1, 3])

    def test_failed_write_is_logged_and_does_not_stick(self):
        control = FakeControl(value=30)
        control.write_error = ControlUnavailable('display went away')
        controller, _ = make_controller([control])
        controller.start()

        with self.assertLogs('smartknob_deck.controller', level='WARNING'):
            controller.handle_state(state(20))
        controller.tick()

        self.assertEqual(control.writes, [])


class PressTest(unittest.TestCase):
    def setUp(self):
        self.volume = FakeControl(key='volume', label='Volume', value=30)
        self.brightness = FakeControl(key='brightness', label='Brightness',
                                      value=50, min_value=5)
        self.controller, self.sent = make_controller([self.volume, self.brightness])

    def test_first_state_only_establishes_a_press_baseline(self):
        self.controller.start()

        self.controller.handle_state(state(15, press_nonce=7))

        self.assertIs(self.controller.active_control, self.volume)
        self.assertEqual(len(self.sent), 1)

    def test_press_switches_control_and_sends_its_config(self):
        self.controller.start()
        self.controller.handle_state(state(15, press_nonce=7))

        self.controller.handle_state(state(15, press_nonce=8))

        self.assertIs(self.controller.active_control, self.brightness)
        self.assertEqual(self.sent[-1].text, 'Brightness')
        self.assertEqual(self.sent[-1].position, 50)
        self.assertEqual(self.controller.value, 50)

    def test_press_ignores_the_position_from_the_old_config(self):
        self.controller.start()
        self.controller.handle_state(state(15, press_nonce=7))

        # Same snapshot position, but it belongs to the volume config we just left
        self.controller.handle_state(state(15, press_nonce=8))

        self.assertEqual(self.brightness.writes, [])
        self.assertEqual(self.volume.writes, [])

    def test_press_wraps_around_the_control_list(self):
        self.controller.start()
        self.controller.handle_state(state(15, press_nonce=1))
        self.controller.handle_state(state(15, press_nonce=2))
        self.controller.handle_state(state(9, press_nonce=3,
                                          config_text='Brightness'))

        self.assertIs(self.controller.active_control, self.volume)

    def test_single_control_ignores_presses(self):
        controller, sent = make_controller([self.volume])
        controller.start()
        controller.handle_state(state(15, press_nonce=1))

        controller.handle_state(state(15, press_nonce=2))

        self.assertIs(controller.active_control, self.volume)
        self.assertEqual(len(sent), 1)


class SyncTest(unittest.TestCase):
    def test_states_from_another_config_are_not_applied(self):
        control = FakeControl(value=30)
        controller, _ = make_controller([control])
        controller.start()

        controller.handle_state(state(3, config_text='Unbounded\nNo detents'))

        self.assertEqual(control.writes, [])

    def test_config_is_resent_when_the_knob_is_out_of_sync(self):
        clock = FakeClock()
        control = FakeControl(value=30)
        controller, sent = make_controller([control], clock=clock)
        controller.start()

        stale = state(3, config_text='On/off\nStrong detent')
        controller.handle_state(stale)
        self.assertEqual(len(sent), 1, 'resend should be rate limited')

        clock.advance(RESEND_INTERVAL_SECONDS)
        with self.assertLogs('smartknob_deck.controller', level='INFO'):
            controller.handle_state(stale)

        self.assertEqual(len(sent), 2)
        self.assertEqual(sent[-1].text, 'Fake')
        self.assertEqual(sent[-1].position, 30)

    def test_external_change_moves_the_knob(self):
        clock = FakeClock()
        control = FakeControl(value=30)
        controller, sent = make_controller([control], clock=clock,
                                           poll_interval=2.0)
        controller.start()

        control.value = 60  # e.g. the user hit the volume keys
        clock.advance(2.0)
        with self.assertLogs('smartknob_deck.controller', level='INFO'):
            controller.tick()

        self.assertEqual(controller.value, 60)
        self.assertEqual(sent[-1].position, 60)
        self.assertEqual(control.writes, [], 'polling should not write back')

    def test_polling_waits_until_rotation_settles(self):
        clock = FakeClock()
        control = FakeControl(value=30)
        controller, sent = make_controller([control], clock=clock,
                                           poll_interval=0)
        controller.start()
        controller.handle_state(state(20))

        control.value = 90
        clock.advance(IDLE_BEFORE_POLL_SECONDS / 2)
        controller.tick()
        self.assertEqual(len(sent), 1)

        clock.advance(IDLE_BEFORE_POLL_SECONDS)
        controller.tick()
        self.assertEqual(len(sent), 2)
        self.assertEqual(controller.value, 90)

    def test_unreadable_control_does_not_break_polling(self):
        clock = FakeClock()
        control = FakeControl(value=30)
        controller, sent = make_controller([control], clock=clock,
                                           poll_interval=0)
        controller.start()
        control.read_error = ControlUnavailable('device busy')

        clock.advance(IDLE_BEFORE_POLL_SECONDS + 1)
        controller.tick()

        self.assertEqual(len(sent), 1)
        self.assertEqual(controller.value, 30)


def setUpModule():
    # Keep the controller's INFO chatter out of the test output; assertLogs
    # still captures what the tests care about.
    logging.getLogger('smartknob_deck').setLevel(logging.WARNING)


if __name__ == '__main__':
    unittest.main()
