#include "apps.h"

// Field order of PB_SmartKnobConfig, used by the positional initializers below:
//   position, sub_position_unit, position_nonce,
//   min_position, max_position, position_width_radians,
//   detent_strength_unit, endstop_strength_unit, snap_point,
//   text[51], detent_positions_count, detent_positions[5],
//   snap_point_bias, led_hue
//
// The hues stay inside the amber/champagne family so the LED ring matches the
// display theme; each app is nudged a little warmer or cooler than its neighbour.

static const char* const FAN_LABELS[] = {"OFF", "LOW", "MID", "HIGH"};
static const char* const LIGHT_LABELS[] = {"OFF", "ON"};

const AppDescriptor APPS[] = {
    {
        "VOLUME",
        "Fine level",
        AppIcon::VOLUME,
        ValueStyle::PERCENT,
        PressAction::MUTE,
        nullptr,
        0,
        {
            30, 0, 0,
            0, 100, 3.6 * PI / 180,
            0.3, 1, 1.1,
            "Volume",
            0, {}, 0,
            30,
        },
    },
    {
        "BRIGHTNESS",
        "Fine level",
        AppIcon::BRIGHTNESS,
        ValueStyle::PERCENT,
        PressAction::NONE,
        nullptr,
        0,
        {
            60, 0, 0,
            0, 100, 3.6 * PI / 180,
            0.2, 1, 1.1,
            "Brightness",
            0, {}, 0,
            38,
        },
    },
    {
        "LIGHT",
        "Hard detent",
        AppIcon::BULB,
        ValueStyle::ONOFF,
        PressAction::TOGGLE,
        LIGHT_LABELS,
        2,
        {
            0, 0, 0,
            0, 1, 60 * PI / 180,
            1.6, 1, 0.55,
            "Light",
            0, {}, 0,
            42,
        },
    },
    {
        "FAN",
        "Four steps",
        AppIcon::FAN,
        ValueStyle::STEPS,
        PressAction::CYCLE,
        FAN_LABELS,
        4,
        {
            0, 0, 0,
            0, 3, 30 * PI / 180,
            1.6, 1, 1.1,
            "Fan",
            0, {}, 0,
            26,
        },
    },
    {
        "SCROLL",
        "Unbounded",
        AppIcon::SCROLL,
        ValueStyle::COUNT,
        PressAction::NONE,
        nullptr,
        0,
        {
            0, 0, 0,
            0, -1,  // max < min: no endstops, spins forever
            10 * PI / 180,
            0.4, 1, 1.1,
            "Scroll",
            0, {}, 0,
            34,
        },
    },
    {
        "TIMER",
        "One hour",
        AppIcon::TIMER,
        ValueStyle::MINUTES,
        PressAction::NONE,
        nullptr,
        0,
        {
            10, 0, 0,
            0, 60, 6 * PI / 180,
            0.6, 1, 1.1,
            "Timer",
            0, {}, 0,
            22,
        },
    },
};

const uint8_t APP_COUNT = sizeof(APPS) / sizeof(APPS[0]);

PB_SmartKnobConfig menuConfig(uint8_t selected) {
    PB_SmartKnobConfig config = {
        selected, 0, 0,
        0, APP_COUNT - 1, 43 * PI / 180,
        1.3, 1, 1.1,
        "Menu",
        0, {}, 0,
        32,
    };
    return config;
}

const char* appStepLabel(const AppDescriptor& app, int32_t position) {
    if (app.step_labels == nullptr) {
        return nullptr;
    }
    if (position < 0 || position >= app.step_label_count) {
        return nullptr;
    }
    return app.step_labels[position];
}
