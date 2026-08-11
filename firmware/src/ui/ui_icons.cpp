#if SK_DISPLAY

#include "ui_icons.h"

#include "ui_theme.h"

namespace ui {

/** Stroked arc around an arbitrary centre, drawn as radial spokes. */
static void strokeArc(TFT_eSprite& spr, float cx, float cy, float radius, float thickness, float start_deg, float end_deg, uint16_t color) {
    if (radius <= 0) {
        return;
    }
    float step = 51.6f / max(1.0f, radius);
    int32_t steps = (int32_t)(fabsf(end_deg - start_deg) / step) + 1;
    float direction = end_deg > start_deg ? step : -step;
    float inner = max(0.0f, radius - thickness / 2);
    float outer = radius + thickness / 2;
    for (int32_t i = 0; i <= steps; i++) {
        float rad = (start_deg + direction * i) * PI / 180;
        float cos_a = cosf(rad);
        float sin_a = sinf(rad);
        spr.drawLine(cx + inner * cos_a, cy - inner * sin_a, cx + outer * cos_a, cy - outer * sin_a, color);
    }
}

/** Two-pixel-wide line, thickened perpendicular to its direction. */
static void stroke(TFT_eSprite& spr, float x0, float y0, float x1, float y1, uint16_t color) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = sqrtf(dx * dx + dy * dy);
    spr.drawLine(x0, y0, x1, y1, color);
    if (len < 0.001f) {
        return;
    }
    float nx = -dy / len;
    float ny = dx / len;
    spr.drawLine(x0 + nx, y0 + ny, x1 + nx, y1 + ny, color);
}

static void iconVolume(TFT_eSprite& spr, int16_t cx, int16_t cy, float s, uint16_t color, float value_unit) {
    float body_left = cx - 0.72f * s;
    float cone_x = cx - 0.30f * s;
    spr.fillRect(body_left, cy - 0.20f * s, cone_x - body_left, 0.40f * s, color);
    spr.fillTriangle(cone_x, cy - 0.20f * s, cone_x, cy + 0.20f * s, cx + 0.06f * s, cy + 0.62f * s, color);
    spr.fillTriangle(cone_x, cy - 0.20f * s, cx + 0.06f * s, cy + 0.62f * s, cx + 0.06f * s, cy - 0.62f * s, color);

    uint8_t waves = value_unit <= 0.005f ? 0 : 1 + (uint8_t)(value_unit * 2.99f);
    for (uint8_t i = 0; i < 3; i++) {
        uint16_t wave_color = i < waves ? color : dim(color, 0.22f);
        strokeArc(spr, cx + 0.06f * s, cy, (0.42f + 0.26f * i) * s, 2, -46, 46, wave_color);
    }
}

static void iconBrightness(TFT_eSprite& spr, int16_t cx, int16_t cy, float s, uint16_t color, float value_unit) {
    spr.fillCircle(cx, cy, 0.34f * s, color);
    spr.drawCircle(cx, cy, 0.34f * s + 2, dim(color, 0.35f));
    float inner = 0.52f * s;
    float outer = inner + (0.16f + 0.26f * value_unit) * s;
    for (uint8_t i = 0; i < 8; i++) {
        float rad = (i * 45 + 22.5f) * PI / 180;
        stroke(spr, cx + inner * cosf(rad), cy - inner * sinf(rad), cx + outer * cosf(rad), cy - outer * sinf(rad), color);
    }
}

static void iconBulb(TFT_eSprite& spr, int16_t cx, int16_t cy, float s, uint16_t color, float value_unit) {
    bool on = value_unit > 0.5f;
    float glass_y = cy - 0.22f * s;
    float glass_r = 0.52f * s;
    if (on) {
        spr.fillCircle(cx, glass_y, glass_r - 1, dim(color, 0.30f));
        for (uint8_t i = 0; i < 6; i++) {
            float rad = (i * 60 + 30) * PI / 180;
            stroke(spr,
                cx + (glass_r + 0.14f * s) * cosf(rad), glass_y - (glass_r + 0.14f * s) * sinf(rad),
                cx + (glass_r + 0.34f * s) * cosf(rad), glass_y - (glass_r + 0.34f * s) * sinf(rad),
                dim(color, 0.75f));
        }
    }
    strokeArc(spr, cx, glass_y, glass_r, 2, -35, 215, color);
    stroke(spr, cx - glass_r * 0.82f, glass_y + glass_r * 0.58f, cx - 0.24f * s, cy + 0.42f * s, color);
    stroke(spr, cx + glass_r * 0.82f, glass_y + glass_r * 0.58f, cx + 0.24f * s, cy + 0.42f * s, color);
    for (uint8_t i = 0; i < 3; i++) {
        float y = cy + (0.42f + 0.17f * i) * s;
        stroke(spr, cx - 0.24f * s, y, cx + 0.24f * s, y, i == 2 ? dim(color, 0.6f) : color);
    }
    // Filament, brighter when lit.
    uint16_t filament = on ? COLOR_SPECULAR : dim(color, 0.4f);
    strokeArc(spr, cx, glass_y + 0.1f * s, 0.20f * s, 1, 20, 160, filament);
}

static void iconFan(TFT_eSprite& spr, int16_t cx, int16_t cy, float s, uint16_t color, float value_unit, float anim_deg) {
    for (uint8_t i = 0; i < 3; i++) {
        float rad = (anim_deg + i * 120) * PI / 180;
        float tip_x = cx + 0.62f * s * cosf(rad);
        float tip_y = cy - 0.62f * s * sinf(rad);
        float left = rad + 0.30f;
        float right = rad - 0.30f;
        spr.fillTriangle(cx, cy,
            cx + 0.66f * s * cosf(left), cy - 0.66f * s * sinf(left),
            cx + 0.66f * s * cosf(right), cy - 0.66f * s * sinf(right),
            color);
        spr.fillCircle(tip_x, tip_y, 0.26f * s, color);
    }
    spr.fillCircle(cx, cy, 0.19f * s, COLOR_BG);
    spr.drawCircle(cx, cy, 0.19f * s, color);
    spr.drawCircle(cx, cy, 0.94f * s, dim(color, 0.30f + 0.4f * value_unit));
}

static void iconScroll(TFT_eSprite& spr, int16_t cx, int16_t cy, float s, uint16_t color, float anim_deg) {
    strokeArc(spr, cx, cy, 0.68f * s, 2, anim_deg + 28, anim_deg + 300, color);
    float rad = (anim_deg + 300) * PI / 180;
    float hx = cx + 0.68f * s * cosf(rad);
    float hy = cy - 0.68f * s * sinf(rad);
    float head = rad - PI / 2;
    float wing = 0.24f * s;
    spr.fillTriangle(
        hx + wing * cosf(head), hy - wing * sinf(head),
        hx + wing * cosf(head + 2.3f), hy - wing * sinf(head + 2.3f),
        hx + wing * cosf(head - 2.3f), hy - wing * sinf(head - 2.3f),
        color);
    spr.fillCircle(cx, cy, 0.13f * s, dim(color, 0.55f));
}

static void iconTimer(TFT_eSprite& spr, int16_t cx, int16_t cy, float s, uint16_t color, float value_unit) {
    strokeArc(spr, cx, cy, 0.78f * s, 2, 0, 360, dim(color, 0.55f));
    for (uint8_t i = 0; i < 12; i++) {
        float rad = i * 30 * PI / 180;
        float inner = (i % 3 == 0) ? 0.58f * s : 0.66f * s;
        spr.drawLine(cx + inner * cosf(rad), cy - inner * sinf(rad),
            cx + 0.72f * s * cosf(rad), cy - 0.72f * s * sinf(rad), color);
    }
    float hand = (90 - 360 * value_unit) * PI / 180;
    stroke(spr, cx, cy, cx + 0.52f * s * cosf(hand), cy - 0.52f * s * sinf(hand), color);
    spr.fillCircle(cx, cy, 0.10f * s, color);
}

void icon(TFT_eSprite& spr, AppIcon which, int16_t cx, int16_t cy, float size, uint16_t color, float value_unit, float anim_deg) {
    if (size < 3) {
        return;
    }
    value_unit = clamp01(value_unit);
    switch (which) {
        case AppIcon::VOLUME:
            iconVolume(spr, cx, cy, size, color, value_unit);
            break;
        case AppIcon::BRIGHTNESS:
            iconBrightness(spr, cx, cy, size, color, value_unit);
            break;
        case AppIcon::BULB:
            iconBulb(spr, cx, cy, size, color, value_unit);
            break;
        case AppIcon::FAN:
            iconFan(spr, cx, cy, size, color, value_unit, anim_deg);
            break;
        case AppIcon::SCROLL:
            iconScroll(spr, cx, cy, size, color, anim_deg);
            break;
        case AppIcon::TIMER:
            iconTimer(spr, cx, cy, size, color, value_unit);
            break;
    }
}

}  // namespace ui

#endif
