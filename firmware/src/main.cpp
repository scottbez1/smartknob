#include <Arduino.h>
#include <SimpleFOC.h>

#if SK_DISPLAY
#include "display_task.h"
#endif

#include "interface_task.h"
#include "motor_task.h"
#include "tlv_sensor.h"

#if SK_DISPLAY
DisplayTask display_task = DisplayTask(1);
#endif
MotorTask motor_task = MotorTask(0);

InterfaceTask interface_task = InterfaceTask(1, motor_task);

// CRGB leds[1];

static QueueHandle_t knob_state_debug_queue;

void setup() {
  Serial.begin(115200);

  motor_task.begin();
  interface_task.begin();

  #if SK_DISPLAY
  display_task.begin();

  // Connect display to motor_task's knob state feed
  motor_task.addListener(display_task.getKnobStateQueue());
  #endif


  // Create a queue and register it with motor_task to print knob state to serial (see loop() below)
  knob_state_debug_queue = xQueueCreate(1, sizeof(KnobState));
  assert(knob_state_debug_queue != NULL);

  motor_task.addListener(knob_state_debug_queue);
}


static KnobState state = {};
uint32_t last_debug;

void loop() {
  // Print any new state, at most 5 times per second
  if (millis() - last_debug > 200 && xQueueReceive(knob_state_debug_queue, &state, portMAX_DELAY) == pdTRUE) {
    Serial.println(state.current_position);
    last_debug = millis();
  }
}