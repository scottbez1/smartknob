#if SK_DISPLAY
#include "display_task.h"
#include "semaphore_guard.h"

#include "font/roboto_light_60.h"

static const uint8_t LEDC_CHANNEL_LCD_BACKLIGHT = 0;

DisplayTask::DisplayTask(const uint8_t task_core) : Task{"Display", 4048, 1, task_core} {
  knob_state_queue_ = xQueueCreate(1, sizeof(KnobState));
  assert(knob_state_queue_ != NULL);

  mutex_ = xSemaphoreCreateMutex();
  assert(mutex_ != NULL);
}

DisplayTask::~DisplayTask() {
  vQueueDelete(knob_state_queue_);
  vSemaphoreDelete(mutex_);
}

static void HSV_to_RGB(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
  int i;
  float f,p,q,t;
  
  h = fmax(0.0, fmin(360.0, h));
  s = fmax(0.0, fmin(100.0, s));
  v = fmax(0.0, fmin(100.0, v));
  
  s /= 100;
  v /= 100;
  
  if(s == 0) {
    // Achromatic (grey)
    *r = *g = *b = round(v*255);
    return;
  }

  h /= 60; // sector 0 to 5
  i = floor(h);
  f = h - i; // factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));
  switch(i) {
    case 0:
      *r = round(255*v);
      *g = round(255*t);
      *b = round(255*p);
      break;
    case 1:
      *r = round(255*q);
      *g = round(255*v);
      *b = round(255*p);
      break;
    case 2:
      *r = round(255*p);
      *g = round(255*v);
      *b = round(255*t);
      break;
    case 3:
      *r = round(255*p);
      *g = round(255*q);
      *b = round(255*v);
      break;
    case 4:
      *r = round(255*t);
      *g = round(255*p);
      *b = round(255*v);
      break;
    default: // case 5:
      *r = round(255*v);
      *g = round(255*p);
      *b = round(255*q);
    }
}

void DisplayTask::run() {
    tft_.begin();
    tft_.invertDisplay(1);
    tft_.setRotation(0);
    tft_.fillScreen(TFT_DARKGREEN);

    ledcSetup(LEDC_CHANNEL_LCD_BACKLIGHT, 5000, 16);
    ledcAttachPin(PIN_LCD_BACKLIGHT, LEDC_CHANNEL_LCD_BACKLIGHT);
    ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, UINT16_MAX);

    spr_.setColorDepth(16);

    if (spr_.createSprite(TFT_WIDTH, TFT_HEIGHT) == nullptr) {
      Serial.println("ERROR: sprite allocation failed!");
      tft_.fillScreen(TFT_RED);
    } else {
      Serial.println("Sprite created!");
      tft_.fillScreen(TFT_PURPLE);
    }
    spr_.setTextColor(0xFFFF, TFT_BLACK);
    
    KnobState state;

    const int RADIUS = TFT_WIDTH / 2;
    const uint16_t FILL_COLOR = spr_.color565(90, 18, 151);
    const uint16_t DOT_COLOR = spr_.color565(80, 100, 200);

    int32_t pointer_center_x = TFT_WIDTH / 2;
    int32_t pointer_center_y = TFT_HEIGHT / 2;
    int32_t pointer_length_short = 10;
    int32_t pointer_length_long = TFT_WIDTH / 2 - 5;

    spr_.setTextDatum(CC_DATUM);
    spr_.setTextColor(TFT_WHITE);
    while(1) {
        if (xQueueReceive(knob_state_queue_, &state, portMAX_DELAY) == pdFALSE) {
          continue;
        }

        spr_.fillSprite(TFT_BLACK);
        if (state.config.num_positions > 1) {
          int32_t height = state.current_position * TFT_HEIGHT / (state.config.num_positions - 1);
          spr_.fillRect(0, TFT_HEIGHT - height, TFT_WIDTH, height, FILL_COLOR);
        }

        spr_.setFreeFont(&Roboto_Light_60);
        spr_.drawString(String() + state.current_position, TFT_WIDTH / 2, TFT_HEIGHT / 2 - VALUE_OFFSET, 1);
        spr_.setFreeFont(&DESCRIPTION_FONT);
        int32_t line_y = TFT_HEIGHT / 2 + DESCRIPTION_Y_OFFSET;
        char* start = state.config.descriptor;
        char* end = start + strlen(state.config.descriptor);
        while (start < end) {
          char* newline = strchr(start, '\n');
          if (newline == nullptr) {
            newline = end;
          }
          
          char buf[sizeof(state.config.descriptor)] = {};
          strncat(buf, start, min(sizeof(buf) - 1, (size_t)(newline - start)));
          spr_.drawString(String(buf), TFT_WIDTH / 2, line_y, 1);
          start = newline + 1;
          line_y += spr_.fontHeight(1);
        }

        float left_bound = PI / 2;

        if (state.config.num_positions > 0) {
          float range_radians = (state.config.num_positions - 1) * state.config.position_width_radians;
          left_bound = PI / 2 + range_radians / 2;
          float right_bound = PI / 2 - range_radians / 2;
          spr_.drawLine(TFT_WIDTH/2 + RADIUS * cosf(left_bound), TFT_HEIGHT/2 - RADIUS * sinf(left_bound), TFT_WIDTH/2 + (RADIUS - 10) * cosf(left_bound), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(left_bound), TFT_WHITE);
          spr_.drawLine(TFT_WIDTH/2 + RADIUS * cosf(right_bound), TFT_HEIGHT/2 - RADIUS * sinf(right_bound), TFT_WIDTH/2 + (RADIUS - 10) * cosf(right_bound), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(right_bound), TFT_WHITE);
        }
        if (DRAW_ARC) {
          spr_.drawCircle(TFT_WIDTH/2, TFT_HEIGHT/2, RADIUS, TFT_DARKGREY);
        }

        float adjusted_sub_position = state.sub_position_unit * state.config.position_width_radians;
        if (state.config.num_positions > 0) {
          if (state.current_position == 0 && state.sub_position_unit < 0) {
            adjusted_sub_position = -logf(1 - state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180) * 5 * PI / 180;
          } else if (state.current_position == state.config.num_positions - 1 && state.sub_position_unit > 0) {
            adjusted_sub_position = logf(1 + state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180)  * 5 * PI / 180;
          }
        }

        float raw_angle = left_bound - state.current_position * state.config.position_width_radians;
        float adjusted_angle = raw_angle - adjusted_sub_position;

        if (state.config.num_positions > 0 && ((state.current_position == 0 && state.sub_position_unit < 0) || (state.current_position == state.config.num_positions - 1 && state.sub_position_unit > 0))) {

          spr_.fillCircle(TFT_WIDTH/2 + (RADIUS - 10) * cosf(raw_angle), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(raw_angle), 5, DOT_COLOR);
          if (raw_angle < adjusted_angle) {
            for (float r = raw_angle; r <= adjusted_angle; r += 2 * PI / 180) {
              spr_.fillCircle(TFT_WIDTH/2 + (RADIUS - 10) * cosf(r), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(r), 2, DOT_COLOR);
            }
            spr_.fillCircle(TFT_WIDTH/2 + (RADIUS - 10) * cosf(adjusted_angle), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(adjusted_angle), 2, DOT_COLOR);
          } else {
            for (float r = raw_angle; r >= adjusted_angle; r -= 2 * PI / 180) {
              spr_.fillCircle(TFT_WIDTH/2 + (RADIUS - 10) * cosf(r), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(r), 2, DOT_COLOR);
            }
            spr_.fillCircle(TFT_WIDTH/2 + (RADIUS - 10) * cosf(adjusted_angle), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(adjusted_angle), 2, DOT_COLOR);
          }
        } else {
          spr_.fillCircle(TFT_WIDTH/2 + (RADIUS - 10) * cosf(adjusted_angle), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(adjusted_angle), 5, DOT_COLOR);
        }

        spr_.pushSprite(0, 0);

        {
          SemaphoreGuard lock(mutex_);
          ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, brightness_);
        }
        delay(2);
    }
}

QueueHandle_t DisplayTask::getKnobStateQueue() {
  return knob_state_queue_;
}

void DisplayTask::setBrightness(uint16_t brightness) {
  SemaphoreGuard lock(mutex_);
  brightness_ = brightness;
}

#endif