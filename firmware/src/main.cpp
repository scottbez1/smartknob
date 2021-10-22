#include <Arduino.h>
#include <TFT_eSPI.h>
#include <FastLED.h>
#include <SimpleFOC.h>

#include "display_task.h"
#include "motor_task.h"
#include "tlv_sensor.h"

DisplayTask display_task = DisplayTask(1);
MotorTask motor_task = MotorTask(0, display_task);

CRGB leds[1];


void setup() {
  Serial.begin(115200);

  display_task.begin();
  motor_task.begin();

  vTaskDelete(nullptr);
}


void loop() {
  assert(false);
}