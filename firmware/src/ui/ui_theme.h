#pragma once

#if SK_DISPLAY

#include <Arduino.h>
#include <TFT_eSPI.h>

/**
 * Shared look and feel: a black face, hairline slate rings and champagne gold
 * accents. The sprite is 8 bits per pixel, so colours are quantised to 3-3-2 on
 * push. The palette below is picked to survive that quantisation without going
 * muddy, and the drawing helpers stay flat (no gradients) to avoid banding.
 */
namespace ui {

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

constexpr uint16_t COLOR_BG = rgb565(0, 0, 0);
constexpr uint16_t COLOR_HAIRLINE = rgb565(70, 70, 78);
constexpr uint16_t COLOR_TRACK = rgb565(105, 78, 26);
constexpr uint16_t COLOR_GOLD = rgb565(230, 190, 90);
constexpr uint16_t COLOR_GOLD_SOFT = rgb565(170, 135, 60);
constexpr uint16_t COLOR_GOLD_DEEP = rgb565(115, 88, 34);
constexpr uint16_t COLOR_SPECULAR = rgb565(255, 244, 205);
constexpr uint16_t COLOR_TEXT = rgb565(250, 246, 236);
constexpr uint16_t COLOR_TEXT_DIM = rgb565(150, 145, 135);

constexpr int16_t CENTER_X = TFT_WIDTH / 2;
constexpr int16_t CENTER_Y = TFT_HEIGHT / 2;
constexpr int16_t BEZEL_RADIUS = TFT_WIDTH / 2 - 2;

/** Sweep of the value arc on an app page: 210 degrees down to -30. */
constexpr float ARC_START_DEG = 210;
constexpr float ARC_SWEEP_DEG = 240;

float clamp01(float value);
float easeOutCubic(float t);
float easeInOutCubic(float t);
/** Overshoots slightly past 1 before settling; used for the confirm pop. */
float easeOutBack(float t);
float lerpf(float a, float b, float t);

/** Scales a 565 colour toward black. */
uint16_t dim(uint16_t color, float factor);
/** Blends two 565 colours; t=0 returns a, t=1 returns b. */
uint16_t mix(uint16_t a, uint16_t b, float t);

/** Filled band between two radii, swept between two angles (degrees, CCW). */
void arc(TFT_eSprite& spr, float radius_inner, float radius_outer, float start_deg, float end_deg, uint16_t color);
/** Hairline circle centred on the face. */
void ring(TFT_eSprite& spr, float radius, uint16_t color);
/** Radial tick, centred on the given angle. */
void tick(TFT_eSprite& spr, float angle_deg, float radius_inner, float radius_outer, float width, uint16_t color);
/** Round dot at a polar position. */
void dot(TFT_eSprite& spr, float angle_deg, float radius, float size, uint16_t color);
/** Concentric bands that fade outward, faking a soft glow around a radius. */
void glow(TFT_eSprite& spr, float radius, float thickness, uint16_t color, float intensity);

/** Letter-spaced, centred text. Tracking is extra pixels between glyphs. */
void trackedText(TFT_eSprite& spr, const char* text, int16_t cx, int16_t cy, const GFXfont* font, uint16_t color, float tracking);
/** Width trackedText() would occupy. */
int16_t trackedTextWidth(TFT_eSprite& spr, const char* text, const GFXfont* font, float tracking);
/** Plain centred text in a free font. */
void centeredText(TFT_eSprite& spr, const char* text, int16_t cx, int16_t cy, const GFXfont* font, uint16_t color);

/**
 * Face furniture drawn under every page: bezel hairlines plus a slow specular
 * highlight that travels around the rim.
 */
void backdrop(TFT_eSprite& spr, uint32_t now_ms, float fade);

}  // namespace ui

#endif
