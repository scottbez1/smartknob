#pragma once

#if SK_DISPLAY

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "../proto_gen/smartknob.pb.h"
#include "ui_state.h"

namespace ui {

/** Called once before the first render, after the sprite exists. */
void renderInit(TFT_eSprite& spr);

/**
 * Draws one frame into the sprite. Animation state is kept internally and
 * advanced from now_ms, so this wants to be called continuously (~60 Hz) rather
 * than only when the knob state changes.
 */
void render(TFT_eSprite& spr, const PB_SmartKnobState& state, const UiState& ui_state, uint32_t now_ms);

}  // namespace ui

#endif
