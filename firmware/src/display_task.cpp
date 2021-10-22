#include "display_task.h"
#include "semaphore_guard.h"

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
    spr_.setFreeFont(&Roboto_Thin_24);
    spr_.setTextColor(0xFFFF, TFT_BLACK);
    
    float angle;

    int32_t pointer_center_x = TFT_WIDTH / 2;
    int32_t pointer_center_y = TFT_HEIGHT * 2 / 3;
    int32_t pointer_length_short = 10;
    int32_t pointer_length_long = TFT_WIDTH / 2;
    while(1) {

        {
            SemaphoreGuard lock(semaphore_);
            angle = angle_;// < 0 ? angle_ + 2 * PI : angle_;
        }
        float degrees = angle * 360 / 2 / PI;

        uint8_t r, g, b;
        HSV_to_RGB(degrees, 80, 80, &r, &g, &b);

        spr_.fillSprite(tft_.color565(r, g, b));
        spr_.setCursor(40, 40);

        spr_.printf("%.1f", degrees);

        float pointer_angle = - angle;
        spr_.fillTriangle(
            pointer_center_x + pointer_length_short * cos(pointer_angle - PI * 3 /4),
            pointer_center_y + pointer_length_short * sin(pointer_angle - PI * 3 /4),
            pointer_center_x + pointer_length_short * cos(pointer_angle + PI * 3 /4),
            pointer_center_y + pointer_length_short * sin(pointer_angle + PI * 3 /4),
            pointer_center_x + pointer_length_long * cos(pointer_angle),
            pointer_center_y + pointer_length_long * sin(pointer_angle),
            TFT_WHITE
        );

        spr_.fillCircle(pointer_center_x, pointer_center_y, 3, TFT_RED);


        spr_.pushSprite(0, 0);
        delay(10);
    }
}

void DisplayTask::set_angle(float angle) {
    SemaphoreGuard lock(semaphore_);
    angle_ = angle;
}
