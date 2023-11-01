#if SK_DISPLAY
#include "display_task.h"
#include "semaphore_guard.h"
#include "util.h"

#include "font/roboto_light_60.h"

static const uint8_t LEDC_CHANNEL_LCD_BACKLIGHT = 0;

DisplayTask::DisplayTask(const uint8_t task_core) : Task{"Display", 2048, 1, task_core} {
  knob_state_queue_ = xQueueCreate(1, sizeof(PB_SmartKnobState));
  assert(knob_state_queue_ != NULL);

  mutex_ = xSemaphoreCreateMutex();
  assert(mutex_ != NULL);
}

DisplayTask::~DisplayTask() {
  vQueueDelete(knob_state_queue_);
  vSemaphoreDelete(mutex_);
}

static void drawPlayButton(TFT_eSprite& spr, int x, int y, int width, int height, uint16_t color) {
  spr.fillTriangle(
    x, y - height / 2,
    x, y + height / 2,
    x + width, y,
    color
  );
}

void DisplayTask::run() {
    tft_.begin();
    tft_.invertDisplay(1);
    tft_.setRotation(SK_DISPLAY_ROTATION);
    tft_.fillScreen(TFT_DARKGREEN);

    ledcSetup(LEDC_CHANNEL_LCD_BACKLIGHT, 5000, SK_BACKLIGHT_BIT_DEPTH);
    ledcAttachPin(PIN_LCD_BACKLIGHT, LEDC_CHANNEL_LCD_BACKLIGHT);
    ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, (1 << SK_BACKLIGHT_BIT_DEPTH) - 1);

    spr_.setColorDepth(8);

    if (spr_.createSprite(TFT_WIDTH, TFT_HEIGHT) == nullptr) {
      log("ERROR: sprite allocation failed!");
      tft_.fillScreen(TFT_RED);
    } else {
      log("Sprite created!");
      tft_.fillScreen(TFT_PURPLE);
    }
    spr_.setTextColor(0xFFFF, TFT_BLACK);
    
    PB_SmartKnobState state;

    const int RADIUS = TFT_WIDTH / 2;
    const uint16_t FILL_COLOR = spr_.color565(90, 18, 151);
    const uint16_t DOT_COLOR = spr_.color565(80, 100, 200);

    spr_.setTextDatum(CC_DATUM);
    spr_.setTextColor(TFT_WHITE);
    while(1) {
        if (xQueueReceive(knob_state_queue_, &state, portMAX_DELAY) == pdFALSE) {
          continue;
        }

        spr_.fillSprite(TFT_BLACK);

        int32_t num_positions = state.config.max_position - state.config.min_position + 1;
        float adjusted_sub_position = state.sub_position_unit * state.config.position_width_radians;
        if (num_positions > 0) {
          if (state.current_position == state.config.min_position && state.sub_position_unit < 0) {
            adjusted_sub_position = -logf(1 - state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180) * 5 * PI / 180;
          } else if (state.current_position == state.config.max_position && state.sub_position_unit > 0) {
            adjusted_sub_position = logf(1 + state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180)  * 5 * PI / 180;
          }
        }

        float left_bound = PI / 2;
        float right_bound = 0;
        if (num_positions > 0) {
          float range_radians = (state.config.max_position - state.config.min_position) * state.config.position_width_radians;
          left_bound = PI / 2 + range_radians / 2;
          right_bound = PI / 2 - range_radians / 2;
        }
        float raw_angle = left_bound - (state.current_position - state.config.min_position) * state.config.position_width_radians;
        float adjusted_angle = raw_angle - adjusted_sub_position;
        
        bool sk_demo_mode = strncmp(state.config.text, "SKDEMO_", 7) == 0;

        if (!sk_demo_mode) {
          if (num_positions > 1) {
            int32_t height = (state.current_position - state.config.min_position) * TFT_HEIGHT / (state.config.max_position - state.config.min_position);
            spr_.fillRect(0, TFT_HEIGHT - height, TFT_WIDTH, height, FILL_COLOR);
          }

          spr_.setFreeFont(&Roboto_Light_60);
          spr_.drawNumber(state.current_position, TFT_WIDTH / 2, TFT_HEIGHT / 2 - VALUE_OFFSET, 1);
          spr_.setFreeFont(&DESCRIPTION_FONT);
          int32_t line_y = TFT_HEIGHT / 2 + DESCRIPTION_Y_OFFSET;
          char* start = state.config.text;
          char* end = start + strlen(state.config.text);
          while (start < end) {
            char* newline = strchr(start, '\n');
            if (newline == nullptr) {
              newline = end;
            }
            
            char buf[sizeof(state.config.text)] = {};
            strncat(buf, start, min(sizeof(buf) - 1, (size_t)(newline - start)));
            spr_.drawString(String(buf), TFT_WIDTH / 2, line_y, 1);
            start = newline + 1;
            line_y += spr_.fontHeight(1);
          }

          if (num_positions > 0) {
            spr_.drawLine(TFT_WIDTH/2 + RADIUS * cosf(left_bound), TFT_HEIGHT/2 - RADIUS * sinf(left_bound), TFT_WIDTH/2 + (RADIUS - 10) * cosf(left_bound), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(left_bound), TFT_WHITE);
            spr_.drawLine(TFT_WIDTH/2 + RADIUS * cosf(right_bound), TFT_HEIGHT/2 - RADIUS * sinf(right_bound), TFT_WIDTH/2 + (RADIUS - 10) * cosf(right_bound), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(right_bound), TFT_WHITE);
          }
          if (DRAW_ARC) {
            spr_.drawCircle(TFT_WIDTH/2, TFT_HEIGHT/2, RADIUS, TFT_DARKGREY);
          }

          if (num_positions > 0 && ((state.current_position == state.config.min_position && state.sub_position_unit < 0) || (state.current_position == state.config.max_position && state.sub_position_unit > 0))) {
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
        } else {
          if (strncmp(state.config.text, "SKDEMO_Scroll", 13) == 0) {
            spr_.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, spr_.color565(150, 0, 0));
            spr_.setFreeFont(&Roboto_Thin_24);
            spr_.drawString("Scroll", TFT_WIDTH / 2, TFT_HEIGHT / 2, 1);
            bool detent = false;
            for (uint8_t i = 0; i < state.config.detent_positions_count; i++) {
              if (state.config.detent_positions[i] == state.current_position) {
                detent = true;
                break;
              }
            }
            spr_.fillCircle(TFT_WIDTH/2 + (RADIUS - 16) * cosf(adjusted_angle), TFT_HEIGHT/2 - (RADIUS - 16) * sinf(adjusted_angle), detent ? 8 : 5, TFT_WHITE);
          } else if (strncmp(state.config.text, "SKDEMO_Frames", 13) == 0) {
            int32_t width = (state.current_position - state.config.min_position) * TFT_WIDTH / (state.config.max_position - state.config.min_position);
            spr_.fillRect(0, 0, width, TFT_HEIGHT, spr_.color565(0, 150, 0));
            spr_.setFreeFont(&Roboto_Light_60);
            spr_.drawNumber(state.current_position, TFT_WIDTH / 2, TFT_HEIGHT / 2, 1);
            spr_.setFreeFont(&Roboto_Thin_24);
            spr_.drawString("Frame", TFT_WIDTH / 2, TFT_HEIGHT / 2 - DESCRIPTION_Y_OFFSET - VALUE_OFFSET, 1);
          } else if (strncmp(state.config.text, "SKDEMO_Speed", 12) == 0) {
            spr_.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, spr_.color565(0, 0, 150));

            float normalizedFractional = sgn(state.sub_position_unit) *
                CLAMP(lerp(state.sub_position_unit * sgn(state.sub_position_unit), 0.1, 0.9, 0, 1), (float)0, (float)1);
            float normalized = state.current_position + normalizedFractional;
            float speed = sgn(normalized) * powf(2, fabsf(normalized) - 1);
            float roundedSpeed = truncf(speed * 10) / 10;

            spr_.setFreeFont(&Roboto_Thin_24);
            if (roundedSpeed == 0) {
              spr_.drawString("Paused", TFT_WIDTH / 2, TFT_HEIGHT / 2 + DESCRIPTION_Y_OFFSET + VALUE_OFFSET, 1);

              spr_.fillRect(TFT_WIDTH / 2 + 5, TFT_HEIGHT / 2 - 20, 10, 40, TFT_WHITE);
              spr_.fillRect(TFT_WIDTH / 2 - 5 - 10, TFT_HEIGHT / 2 - 20, 10, 40, TFT_WHITE);
            } else {
              char buf[10];
              snprintf(buf, sizeof(buf), "%0.1fx", roundedSpeed);
              spr_.drawString(buf, TFT_WIDTH / 2, TFT_HEIGHT / 2 + DESCRIPTION_Y_OFFSET + VALUE_OFFSET, 1);

              uint16_t x = TFT_WIDTH / 2;
              for (uint8_t i = 0; i < max(1, abs(state.current_position)); i++) {
                drawPlayButton(spr_, x, TFT_HEIGHT / 2, sgn(roundedSpeed) * 20, 40, TFT_WHITE);
                x += sgn(roundedSpeed) * 20;
              }
            }
          }
        }

        spr_.pushSprite(0, 0);

        {
          SemaphoreGuard lock(mutex_);
          ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, brightness_);
        }
        delay(5);
    }
}

QueueHandle_t DisplayTask::getKnobStateQueue() {
  return knob_state_queue_;
}

void DisplayTask::setBrightness(uint16_t brightness) {
  SemaphoreGuard lock(mutex_);
  brightness_ = brightness >> (16 - SK_BACKLIGHT_BIT_DEPTH);
}

void DisplayTask::setLogger(Logger* logger) {
    logger_ = logger;
}

void DisplayTask::log(const char* msg) {
    if (logger_ != nullptr) {
        logger_->log(msg);
    }
}

#endif