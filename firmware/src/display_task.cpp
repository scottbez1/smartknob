#if SK_DISPLAY
#include "display_task.h"
#include "semaphore_guard.h"

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

void DisplayTask::run() {
    tft_.begin();
    tft_.invertDisplay(1);
    tft_.setRotation(SK_DISPLAY_ROTATION);
    tft_.fillScreen(TFT_DARKGREEN);

    ledcSetup(LEDC_CHANNEL_LCD_BACKLIGHT, 5000, 16);
    ledcAttachPin(PIN_LCD_BACKLIGHT, LEDC_CHANNEL_LCD_BACKLIGHT);
    ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, UINT16_MAX);

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
        if (num_positions > 1) {
          int32_t height = (state.current_position - state.config.min_position) * TFT_HEIGHT / (state.config.max_position - state.config.min_position);
          spr_.fillRect(0, TFT_HEIGHT - height, TFT_WIDTH, height, FILL_COLOR);
        }

        spr_.setFreeFont(&Roboto_Light_60);
        spr_.drawString(String() + state.current_position, TFT_WIDTH / 2, TFT_HEIGHT / 2 - VALUE_OFFSET, 1);
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

        float left_bound = PI / 2;

        if (num_positions > 0) {
          float range_radians = (state.config.max_position - state.config.min_position) * state.config.position_width_radians;
          left_bound = PI / 2 + range_radians / 2;
          float right_bound = PI / 2 - range_radians / 2;
          spr_.drawLine(TFT_WIDTH/2 + RADIUS * cosf(left_bound), TFT_HEIGHT/2 - RADIUS * sinf(left_bound), TFT_WIDTH/2 + (RADIUS - 10) * cosf(left_bound), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(left_bound), TFT_WHITE);
          spr_.drawLine(TFT_WIDTH/2 + RADIUS * cosf(right_bound), TFT_HEIGHT/2 - RADIUS * sinf(right_bound), TFT_WIDTH/2 + (RADIUS - 10) * cosf(right_bound), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(right_bound), TFT_WHITE);
        }
        if (DRAW_ARC) {
          spr_.drawCircle(TFT_WIDTH/2, TFT_HEIGHT/2, RADIUS, TFT_DARKGREY);
        }

        float adjusted_sub_position = state.sub_position_unit * state.config.position_width_radians;
        if (num_positions > 0) {
          if (state.current_position == state.config.min_position && state.sub_position_unit < 0) {
            adjusted_sub_position = -logf(1 - state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180) * 5 * PI / 180;
          } else if (state.current_position == state.config.max_position && state.sub_position_unit > 0) {
            adjusted_sub_position = logf(1 + state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180)  * 5 * PI / 180;
          }
        }

        float raw_angle = left_bound - (state.current_position - state.config.min_position) * state.config.position_width_radians;
        float adjusted_angle = raw_angle - adjusted_sub_position;

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

void DisplayTask::setLogger(Logger* logger) {
    logger_ = logger;
}

void DisplayTask::log(const char* msg) {
    if (logger_ != nullptr) {
        logger_->log(msg);
    }
}

#endif