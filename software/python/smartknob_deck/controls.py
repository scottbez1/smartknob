"""
Controls that a SmartKnob can adjust on the host computer.

Each Control maps an integer "value" (e.g. volume percent) onto the knob's
integer positions, and knows how to read/write that value on the OS. Backend
imports are done lazily inside the methods so that this module can be imported
(and unit tested) on any platform, with or without the optional dependencies
installed.
"""

import logging
import math

logger = logging.getLogger(__name__)


class ControlUnavailable(Exception):
    """Raised when a control can't be used on this machine (missing dep/device)."""


class Control:
    """
    Base class for something the knob can adjust.

    Subclasses set the class attributes below and implement _read()/_write().
    """

    # Short name used on the command line
    key = None
    # Shown on the knob's display, above the current value
    label = None
    # Hue (0-255) for the LED ring while this control is active
    led_hue = 0

    # Inclusive range of values. Knob positions map 1:1 onto values, so one
    # detent is one unit and the number on the knob's display is the real value.
    min_value = 0
    max_value = 100

    # Total knob rotation between the two endstops, the same for every control
    # no matter how many values it has. The firmware centres a bounded range on
    # the top of the display (display_task.cpp draws it from
    # PI/2 + range/2 to PI/2 - range/2), so 300 degrees puts the minimum at
    # 150 degrees counter-clockwise from top, the maximum 150 degrees clockwise,
    # and leaves a 60 degree gap centred on the bottom.
    total_travel_degrees = 300

    # Feel of the knob for this control
    detent_strength_unit = 0.6
    endstop_strength_unit = 1
    snap_point = 1.1

    @property
    def position_width_radians(self):
        """
        Angular width of one detent, derived so that min_value..max_value always
        sweeps total_travel_degrees.
        """
        span = self.max_value - self.min_value
        if span <= 0:
            return math.radians(self.total_travel_degrees)
        return math.radians(self.total_travel_degrees / span)

    # Minimum seconds between writes to the OS; rotating faster than this
    # coalesces into a single write of the latest value.
    min_write_interval = 0

    def clamp(self, value):
        return max(self.min_value, min(self.max_value, int(round(value))))

    # Positions are values, but keep the conversion explicit at the call sites
    # in case a future control needs a different mapping.
    value_for_position = clamp
    position_for_value = clamp

    def read(self):
        """Current value from the OS."""
        return self.clamp(self._read())

    def write(self, value):
        """Apply a value to the OS."""
        self._write(self.clamp(value))

    def _read(self):
        raise NotImplementedError

    def _write(self, value):
        raise NotImplementedError


class WindowsVolumeControl(Control):
    """System output volume, via the Core Audio endpoint volume interface."""

    key = 'volume'
    label = 'Volume'
    led_hue = 96  # green

    # 101 detents over the 300 degree sweep, so 3 degrees per percent
    detent_strength_unit = 0.5

    def __init__(self):
        self._endpoint = None

    def _volume(self):
        if self._endpoint is not None:
            return self._endpoint

        try:
            try:
                from pycaw.utils import AudioUtilities
            except ImportError:
                # pycaw < 20230407 kept everything in one module
                from pycaw.pycaw import AudioUtilities
        except ImportError as e:
            raise ControlUnavailable(f'volume control needs pycaw ({e})') from e

        try:
            speakers = AudioUtilities.GetSpeakers()
            endpoint = getattr(speakers, 'EndpointVolume', None)
            if endpoint is None:
                # Older pycaw hands back a raw IMMDevice to activate ourselves
                endpoint = self._activate_endpoint(speakers)
            self._endpoint = endpoint
        except ControlUnavailable:
            raise
        except Exception as e:
            raise ControlUnavailable(f'no usable audio output device ({e})') from e

        return self._endpoint

    @staticmethod
    def _activate_endpoint(speakers):
        try:
            from ctypes import cast, POINTER
            from comtypes import CLSCTX_ALL
            try:
                from pycaw.api.endpointvolume import IAudioEndpointVolume
            except ImportError:
                from pycaw.pycaw import IAudioEndpointVolume
        except ImportError as e:
            raise ControlUnavailable(
                f'volume control needs pycaw and comtypes ({e})') from e
        interface = speakers.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
        return cast(interface, POINTER(IAudioEndpointVolume))

    def _read(self):
        # Scalar (rather than dB) matches what the Windows volume slider shows
        return int(round(self._volume().GetMasterVolumeLevelScalar() * 100))

    def _write(self, value):
        volume = self._volume()
        volume.SetMasterVolumeLevelScalar(value / 100.0, None)
        # Turning the knob up should be audible even if the host was muted
        if value > 0 and volume.GetMute():
            volume.SetMute(0, None)


class WindowsBrightnessControl(Control):
    """Display backlight brightness, via WMI (laptop panels) or DDC/CI."""

    key = 'brightness'
    label = 'Brightness'
    led_hue = 32  # amber

    # Don't allow a fully dark screen. Writes can be slow (especially over
    # DDC/CI to an external monitor), so they're throttled and coalesced.
    min_value = 5
    detent_strength_unit = 1
    min_write_interval = 0.15

    def __init__(self, display=None):
        self._display = display

    def _sbc(self):
        try:
            import screen_brightness_control as sbc
        except ImportError as e:
            raise ControlUnavailable(
                f'brightness control needs screen-brightness-control ({e})') from e
        self._quiet_sbc_logging()
        return sbc

    @staticmethod
    def _quiet_sbc_logging():
        """
        screen-brightness-control re-enumerates displays on every read and
        write, warning each time about panels it can't parse an EDID for or that
        are asleep. Those are expected on laptops with a second display and
        don't stop the brightness we do control from working, but at one pair of
        warnings per poll they bury everything else. Drop them to ERROR unless
        the user asked for debug logging.
        """
        if logging.getLogger('smartknob_deck').isEnabledFor(logging.DEBUG):
            return
        logging.getLogger('screen_brightness_control').setLevel(logging.ERROR)

    def _read(self):
        sbc = self._sbc()
        try:
            values = sbc.get_brightness(display=self._display)
        except Exception as e:
            raise ControlUnavailable(f'no controllable display found ({e})') from e
        if not values:
            raise ControlUnavailable('no controllable display found')
        return int(round(sum(values) / len(values)))

    def _write(self, value):
        sbc = self._sbc()
        try:
            sbc.set_brightness(value, display=self._display)
        except Exception as e:
            raise ControlUnavailable(f'failed to set brightness ({e})') from e


ALL_CONTROLS = {
    WindowsVolumeControl.key: WindowsVolumeControl,
    WindowsBrightnessControl.key: WindowsBrightnessControl,
}


def build_controls(keys):
    """
    Instantiate the named controls, dropping (with a warning) any that can't
    be used on this machine. Raises ControlUnavailable if none are usable.
    """
    controls = []
    for key in keys:
        try:
            factory = ALL_CONTROLS[key]
        except KeyError:
            raise ControlUnavailable(
                f'unknown control "{key}"; choose from: '
                f'{", ".join(sorted(ALL_CONTROLS))}') from None
        control = factory()
        try:
            value = control.read()
        except ControlUnavailable as e:
            logger.warning('Skipping %s control: %s', key, e)
            continue
        logger.info('%s control ready (currently %d)', control.label, value)
        controls.append(control)

    if not controls:
        raise ControlUnavailable('none of the requested controls are usable here')
    return controls
