#include <Arduino.h>
#include <SimpleFOC.h>

#if SK_STRAIN
#include <HX711.h>
#endif

#if SK_LEDS
#include <FastLED.h>
#endif

#if SK_DISPLAY
#include "display_task.h"
#endif

#include "interface_task.h"
#include "motor_task.h"
#include "tlv_sensor.h"
#include "util.h"

#if SK_DISPLAY
DisplayTask display_task = DisplayTask(1);
#endif
MotorTask motor_task = MotorTask(0);

InterfaceTask interface_task = InterfaceTask(1, motor_task);

#if SK_LEDS
CRGB leds[NUM_LEDS];
#endif

#if SK_STRAIN
HX711 scale;
#endif

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

  #if SK_LEDS
  FastLED.addLeds<SK6812, PIN_LED_DATA, GRB>(leds, NUM_LEDS);
  #endif

  #if SK_STRAIN
  scale.begin(38, 2);
  #endif
}


static KnobState state = {};
uint32_t last_debug;

void loop() {
  // Print any new state, at most 5 times per second
  if (millis() - last_debug > 200 && xQueueReceive(knob_state_debug_queue, &state, portMAX_DELAY) == pdTRUE) {
    Serial.println(state.current_position);
    last_debug = millis();
  }

  #if SK_STRAIN
  if (scale.wait_ready_timeout(100)) {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
    Serial.println(reading);
    long lower = 950000;
    long upper = 2500000;
    long value = CLAMP(reading, lower, upper);
    float unit = 1. * (value - lower) / (upper - lower);

    #if SK_LEDS
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      leds[i].setHSV(255 * unit, 255, 128);
    }
    FastLED.show();
    #endif

  } else {
    Serial.println("HX711 not found.");

    #if SK_LEDS
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Red;
    }
    FastLED.show();
    #endif
  }
  #endif
}