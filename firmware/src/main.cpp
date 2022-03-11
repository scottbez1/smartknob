#include <Arduino.h>
#include <SimpleFOC.h>

#include "display_task.h"
#include "interface_task.h"
#include "motor_task.h"

#if SK_DISPLAY
static DisplayTask display_task = DisplayTask(0);
static DisplayTask* display_task_p = &display_task;
#else
static DisplayTask* display_task_p = nullptr;
#endif
static MotorTask motor_task = MotorTask(1);


InterfaceTask interface_task = InterfaceTask(0, motor_task, display_task_p);

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

  // Free up the loop task
  vTaskDelete(NULL);
}


static KnobState state = {};
uint32_t last_debug;

void loop() {
  // Print any new state, at most 5 times per second
  if (millis() - last_debug > 200 && xQueueReceive(knob_state_debug_queue, &state, portMAX_DELAY) == pdTRUE) {
    Serial.println(state.current_position);
    last_debug = millis();
  }

  static uint32_t last_stack_debug;
  if (millis() - last_stack_debug > 1000) {
    Serial.println("Stack high water:");
    Serial.printf("main: %d\n", uxTaskGetStackHighWaterMark(NULL));
    #if SK_DISPLAY
      Serial.printf("display: %d\n", uxTaskGetStackHighWaterMark(display_task.getHandle()));
    #endif
    Serial.printf("motor: %d\n", uxTaskGetStackHighWaterMark(motor_task.getHandle()));
    Serial.printf("interface: %d\n", uxTaskGetStackHighWaterMark(interface_task.getHandle()));
    last_stack_debug = millis();
  }
}