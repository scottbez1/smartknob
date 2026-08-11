#pragma once

#if SK_DISPLAY

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "apps.h"

namespace ui {

/**
 * Draws an app glyph as vector art so it can be scaled freely by the carousel
 * and the page transitions.
 *
 * size       nominal half-extent of the glyph box, in pixels
 * value_unit 0-1 state of the app, for glyphs that reflect their value
 * anim_deg   rotation phase for glyphs that spin
 */
void icon(TFT_eSprite& spr, AppIcon which, int16_t cx, int16_t cy, float size, uint16_t color, float value_unit, float anim_deg);

}  // namespace ui

#endif
