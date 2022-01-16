#include <Arduino.h>
#include <FastLED.h>
#include <SimpleFOC.h>
#include <TFT_eSPI.h>

#include "display_task.h"
#include "interface_task.h"
#include "motor_task.h"
#include "tlv_sensor.h"

DisplayTask display_task = DisplayTask(1);
MotorTask motor_task = MotorTask(0, display_task);
InterfaceTask interface_task = InterfaceTask(1, motor_task);

CRGB leds[1];


void setup() {
  Serial.begin(115200);

  motor_task.begin();
  interface_task.begin();
  display_task.begin();

  vTaskDelete(nullptr);
}


void loop() {
  assert(false);
}