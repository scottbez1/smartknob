#if SK_DISPLAY
#include "display_task.h"
#include "semaphore_guard.h"
#include "util.h"

#include "ui/ui_render.h"

static const uint8_t LEDC_CHANNEL_LCD_BACKLIGHT = 0;

DisplayTask::DisplayTask(const uint8_t task_core) : Task{"Display", 4096, 1, task_core} {
  knob_state_queue_ = xQueueCreate(1, sizeof(PB_SmartKnobState));
  assert(knob_state_queue_ != NULL);

  mutex_ = xSemaphoreCreateMutex();
  assert(mutex_ != NULL);

  PB_SmartKnobState initial_state = PB_SmartKnobState_init_zero;
  state_ = initial_state;
}

DisplayTask::~DisplayTask() {
  vQueueDelete(knob_state_queue_);
  vSemaphoreDelete(mutex_);
}

void DisplayTask::run() {
    tft_.begin();
    tft_.invertDisplay(1);
    tft_.setRotation(SK_DISPLAY_ROTATION);
    tft_.fillScreen(TFT_BLACK);

    ledcSetup(LEDC_CHANNEL_LCD_BACKLIGHT, 5000, SK_BACKLIGHT_BIT_DEPTH);
    ledcAttachPin(PIN_LCD_BACKLIGHT, LEDC_CHANNEL_LCD_BACKLIGHT);
    ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, (1 << SK_BACKLIGHT_BIT_DEPTH) - 1);

    spr_.setColorDepth(8);

    if (spr_.createSprite(TFT_WIDTH, TFT_HEIGHT) == nullptr) {
      log("ERROR: sprite allocation failed!");
      tft_.fillScreen(TFT_RED);
    } else {
      log("Sprite created!");
    }

    ui::renderInit(spr_);

    while(1) {
        // The UI animates on its own (transitions, ripples, the specular sweep),
        // so pull the latest knob state without blocking and draw every frame.
        PB_SmartKnobState new_state;
        if (xQueueReceive(knob_state_queue_, &new_state, 0) == pdTRUE) {
          state_ = new_state;
        }

        UiState ui_state;
        {
          SemaphoreGuard lock(mutex_);
          ui_state = ui_state_;
        }

        ui::render(spr_, state_, ui_state, millis());
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

void DisplayTask::setUiState(const UiState& ui_state) {
  SemaphoreGuard lock(mutex_);
  ui_state_ = ui_state;
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
