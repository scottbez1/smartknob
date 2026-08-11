#if SK_DISPLAY

#include "ui_theme.h"

namespace ui {

float clamp01(float value) {
    if (value < 0) {
        return 0;
    }
    if (value > 1) {
        return 1;
    }
    return value;
}

float easeOutCubic(float t) {
    t = clamp01(t);
    float inv = 1 - t;
    return 1 - inv * inv * inv;
}

float easeInOutCubic(float t) {
    t = clamp01(t);
    if (t < 0.5f) {
        return 4 * t * t * t;
    }
    float inv = -2 * t + 2;
    return 1 - inv * inv * inv / 2;
}

float easeOutBack(float t) {
    t = clamp01(t);
    const float c1 = 1.70158f;
    const float c3 = c1 + 1;
    float inv = t - 1;
    return 1 + c3 * inv * inv * inv + c1 * inv * inv;
}

float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

uint16_t dim(uint16_t color, float factor) {
    factor = factor < 0 ? 0 : factor;
    uint8_t r = ((color >> 11) & 0x1F) * 255 / 31;
    uint8_t g = ((color >> 5) & 0x3F) * 255 / 63;
    uint8_t b = (color & 0x1F) * 255 / 31;
    return rgb565(
        (uint8_t)min(255.0f, r * factor),
        (uint8_t)min(255.0f, g * factor),
        (uint8_t)min(255.0f, b * factor));
}

uint16_t mix(uint16_t a, uint16_t b, float t) {
    t = clamp01(t);
    uint8_t ar = ((a >> 11) & 0x1F) * 255 / 31;
    uint8_t ag = ((a >> 5) & 0x3F) * 255 / 63;
    uint8_t ab = (a & 0x1F) * 255 / 31;
    uint8_t br = ((b >> 11) & 0x1F) * 255 / 31;
    uint8_t bg = ((b >> 5) & 0x3F) * 255 / 63;
    uint8_t bb = (b & 0x1F) * 255 / 31;
    return rgb565(
        (uint8_t)lerpf(ar, br, t),
        (uint8_t)lerpf(ag, bg, t),
        (uint8_t)lerpf(ab, bb, t));
}

void arc(TFT_eSprite& spr, float radius_inner, float radius_outer, float start_deg, float end_deg, uint16_t color) {
    if (radius_outer <= 0 || start_deg == end_deg) {
        return;
    }
    // Step so that neighbouring spokes land within a pixel of each other at the
    // outer edge, which keeps thick arcs solid without wasting draw calls.
    float step = 51.6f / max(1.0f, radius_outer);
    float span = end_deg - start_deg;
    if (fabsf(span) < step) {
        step = fabsf(span);
    }
    int32_t steps = (int32_t)(fabsf(span) / step) + 1;
    float direction = span > 0 ? step : -step;
    for (int32_t i = 0; i <= steps; i++) {
        float angle = start_deg + direction * i;
        if ((direction > 0 && angle > end_deg) || (direction < 0 && angle < end_deg)) {
            angle = end_deg;
        }
        float rad = angle * PI / 180;
        float cos_a = cosf(rad);
        float sin_a = sinf(rad);
        spr.drawLine(
            CENTER_X + radius_inner * cos_a, CENTER_Y - radius_inner * sin_a,
            CENTER_X + radius_outer * cos_a, CENTER_Y - radius_outer * sin_a,
            color);
    }
}

void ring(TFT_eSprite& spr, float radius, uint16_t color) {
    spr.drawCircle(CENTER_X, CENTER_Y, radius, color);
}

void tick(TFT_eSprite& spr, float angle_deg, float radius_inner, float radius_outer, float width, uint16_t color) {
    float half_span = degrees(width / 2 / max(1.0f, radius_outer));
    arc(spr, radius_inner, radius_outer, angle_deg - half_span, angle_deg + half_span, color);
}

void dot(TFT_eSprite& spr, float angle_deg, float radius, float size, uint16_t color) {
    float rad = angle_deg * PI / 180;
    float x = CENTER_X + radius * cosf(rad);
    float y = CENTER_Y - radius * sinf(rad);
    if (size <= 1) {
        spr.drawPixel(x, y, color);
    } else {
        spr.fillCircle(x, y, size, color);
    }
}

void glow(TFT_eSprite& spr, float radius, float thickness, uint16_t color, float intensity) {
    if (intensity <= 0.01f) {
        return;
    }
    for (uint8_t i = 3; i >= 1; i--) {
        float spread = thickness * i;
        float falloff = intensity * 0.30f / i;
        spr.drawCircle(CENTER_X, CENTER_Y, radius + spread, dim(color, falloff));
        spr.drawCircle(CENTER_X, CENTER_Y, radius - spread, dim(color, falloff));
    }
    spr.drawCircle(CENTER_X, CENTER_Y, radius, dim(color, intensity));
}

int16_t trackedTextWidth(TFT_eSprite& spr, const char* text, const GFXfont* font, float tracking) {
    spr.setFreeFont(font);
    size_t len = strlen(text);
    float width = 0;
    for (size_t i = 0; i < len; i++) {
        char buf[2] = {text[i], 0};
        width += spr.textWidth(buf, 1);
        if (i + 1 < len) {
            width += tracking;
        }
    }
    return (int16_t)width;
}

void trackedText(TFT_eSprite& spr, const char* text, int16_t cx, int16_t cy, const GFXfont* font, uint16_t color, float tracking) {
    float x = cx - trackedTextWidth(spr, text, font, tracking) / 2.0f;
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(color);
    size_t len = strlen(text);
    for (size_t i = 0; i < len; i++) {
        char buf[2] = {text[i], 0};
        spr.drawString(buf, x, cy, 1);
        x += spr.textWidth(buf, 1) + tracking;
    }
}

void centeredText(TFT_eSprite& spr, const char* text, int16_t cx, int16_t cy, const GFXfont* font, uint16_t color) {
    spr.setFreeFont(font);
    spr.setTextDatum(CC_DATUM);
    spr.setTextColor(color);
    spr.drawString(text, cx, cy, 1);
}

void backdrop(TFT_eSprite& spr, uint32_t now_ms, float fade) {
    fade = clamp01(fade);
    if (fade <= 0.01f) {
        return;
    }

    ring(spr, BEZEL_RADIUS, dim(COLOR_HAIRLINE, 0.55f * fade));
    ring(spr, BEZEL_RADIUS - 5, dim(COLOR_HAIRLINE, 0.22f * fade));

    // Engine-turned rim: a fine tick every 7.5 degrees.
    uint16_t rim_color = dim(COLOR_GOLD_DEEP, 0.45f * fade);
    for (uint8_t i = 0; i < 48; i++) {
        tick(spr, i * 7.5f, BEZEL_RADIUS - 4, BEZEL_RADIUS - 1, 1, rim_color);
    }

    // Specular highlight travelling slowly around the rim, with a short tail.
    float head = -(float)(now_ms % 14000) * 360 / 14000;
    for (uint8_t i = 0; i < 9; i++) {
        float t = 1 - i / 9.0f;
        arc(spr, BEZEL_RADIUS - 4, BEZEL_RADIUS,
            head + i * 4.5f, head + (i + 1) * 4.5f,
            dim(COLOR_SPECULAR, 0.5f * t * t * fade));
    }
}

}  // namespace ui

#endif
