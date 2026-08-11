#pragma once

#include <Arduino.h>

#include "../proto_gen/smartknob.pb.h"

/** Vector glyph drawn for an app (see ui_icons.h). */
enum class AppIcon : uint8_t {
    VOLUME,
    BRIGHTNESS,
    BULB,
    FAN,
    SCROLL,
    TIMER,
};

/** How the app's position is presented on its page. */
enum class ValueStyle : uint8_t {
    PERCENT,  // 0-100 with a "%" suffix
    ONOFF,    // two positions, shown as ON / OFF
    STEPS,    // discrete positions, shown using step_labels
    COUNT,    // raw position, no unit
    MINUTES,  // raw position with a "MIN" suffix
};

/** What a short press does while the app page is open. */
enum class PressAction : uint8_t {
    NONE,    // visual acknowledgement only
    TOGGLE,  // flip between min and max position
    CYCLE,   // advance one position, wrapping at max
    MUTE,    // jump to min, remembering the previous position
};

struct AppDescriptor {
    const char* name;
    const char* caption;
    AppIcon icon;
    ValueStyle value_style;
    PressAction press_action;
    const char* const* step_labels;
    uint8_t step_label_count;
    PB_SmartKnobConfig config;
};

extern const AppDescriptor APPS[];
extern const uint8_t APP_COUNT;

/** Upper bound on APP_COUNT, so callers can size per-app arrays statically. */
constexpr uint8_t APP_SLOTS = 8;

/** Haptic config for the menu carousel itself, opened at the given selection. */
PB_SmartKnobConfig menuConfig(uint8_t selected);

/** Label for a position of a STEPS/ONOFF app, or nullptr if the style has no labels. */
const char* appStepLabel(const AppDescriptor& app, int32_t position);
