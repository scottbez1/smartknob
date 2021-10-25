#include "display_task.h"
#include "semaphore_guard.h"

#include "font/roboto_light_60.h"

DisplayTask::DisplayTask(const uint8_t task_core) : Task{"Display", 8192, 1, task_core} {
    semaphore_ = xSemaphoreCreateMutex();
    assert(semaphore_ != NULL);
    xSemaphoreGive(semaphore_);
}

DisplayTask::~DisplayTask() {
    if (semaphore_ != NULL) {
        vSemaphoreDelete(semaphore_);
    }
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

    spr_.setColorDepth(16);
    spr_.createSprite(TFT_WIDTH, TFT_HEIGHT);
    spr_.setFreeFont(&Roboto_Light_60);
    spr_.setTextColor(0xFFFF, TFT_BLACK);
    
    KnobState state;

    const int RADIUS = TFT_WIDTH / 2;

    int32_t pointer_center_x = TFT_WIDTH / 2;
    int32_t pointer_center_y = TFT_HEIGHT / 2;
    int32_t pointer_length_short = 10;
    int32_t pointer_length_long = TFT_WIDTH / 2 - 5;

    spr_.setTextDatum(CC_DATUM);
    spr_.setTextColor(TFT_WHITE);
    while(1) {

        {
            SemaphoreGuard lock(semaphore_);
            state = state_;
        }
        // float degrees = angle * 360 / 2 / PI;

        // uint8_t r, g, b;
        // HSV_to_RGB(degrees, 80, 80, &r, &g, &b);

        // spr_.fillSprite(tft_.color565(r, g, b));
        spr_.fillSprite(TFT_BLACK);

        spr_.drawString(String() + state.current_position, TFT_WIDTH / 2, TFT_HEIGHT / 2, 1);
        // spr_.setCursor(60, 40);
        // spr_.printf("%.1f", degrees);

        // float pointer_angle = - angle;
        // spr_.fillTriangle(
        //     pointer_center_x + pointer_length_short * cos(pointer_angle - PI * 3 /4),
        //     pointer_center_y + pointer_length_short * sin(pointer_angle - PI * 3 /4),
        //     pointer_center_x + pointer_length_short * cos(pointer_angle + PI * 3 /4),
        //     pointer_center_y + pointer_length_short * sin(pointer_angle + PI * 3 /4),
        //     pointer_center_x + pointer_length_long * cos(pointer_angle),
        //     pointer_center_y + pointer_length_long * sin(pointer_angle),
        //     TFT_WHITE
        // );

        // spr_.fillCircle(pointer_center_x, pointer_center_y, 3, TFT_RED);
        float left_bound = PI / 2;

        if (state.num_positions > 0) {
          float range_radians = (state.num_positions - 1) * state.position_width_radians;
          left_bound = PI / 2 + range_radians / 2;
          float right_bound = PI / 2 - range_radians / 2;
          spr_.drawLine(TFT_WIDTH/2 + RADIUS * cosf(left_bound), TFT_HEIGHT/2 - RADIUS * sinf(left_bound), TFT_WIDTH/2 + (RADIUS - 10) * cosf(left_bound), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(left_bound), TFT_WHITE);
          spr_.drawLine(TFT_WIDTH/2 + RADIUS * cosf(right_bound), TFT_HEIGHT/2 - RADIUS * sinf(right_bound), TFT_WIDTH/2 + (RADIUS - 10) * cosf(right_bound), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(right_bound), TFT_WHITE);
        }

        float angle = left_bound - (state.sub_position_unit + state.current_position) * state.position_width_radians;
        spr_.fillCircle(TFT_WIDTH/2 + (RADIUS - 10) * cosf(angle), TFT_HEIGHT/2 - (RADIUS - 10) * sinf(angle), 5, TFT_BLUE);


        spr_.pushSprite(0, 0);
        delay(2);
    }
}

void DisplayTask::setData(KnobState state) {
    SemaphoreGuard lock(semaphore_);
    state_ = state;
}
