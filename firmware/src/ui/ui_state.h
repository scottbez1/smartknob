#pragma once

#include <Arduino.h>

enum class UiMode : uint8_t {
    /** Rotate to browse the app carousel, press to open. */
    MENU,
    /** A local app page is open; hold to go back to the menu. */
    APP,
    /** A config arrived over serial, so a host is driving the knob. */
    REMOTE,
};

struct UiState {
    UiMode mode = UiMode::MENU;
    /** Index into APPS while mode == APP. */
    uint8_t app_index = 0;
    /** 0-1 progress of the press-and-hold that returns to the menu. */
    float hold_progress = 0;
    /** Bumped on every mode change so the renderer can start a transition. */
    uint8_t mode_nonce = 0;
    /** Bumped on every short press so the renderer can play a tap ripple. */
    uint8_t press_nonce = 0;
};
