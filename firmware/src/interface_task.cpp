#include <AceButton.h>

#if SK_LEDS
#include <FastLED.h>
#endif

#if SK_STRAIN
#include <HX711.h>
#endif

#if SK_ALS
#include <Adafruit_VEML7700.h>
#endif

#include "interface_task.h"
#include "util.h"

using namespace ace_button;

#define COUNT_OF(A) (sizeof(A) / sizeof(A[0]))

#if SK_LEDS
CRGB leds[NUM_LEDS];
#endif

#if SK_STRAIN
HX711 scale;
#endif

#if SK_ALS
Adafruit_VEML7700 veml = Adafruit_VEML7700();
#endif

static KnobConfig configs[] = {
    // int32_t num_positions;
    // int32_t position;
    // float position_width_radians;
    // float detent_strength_unit;
    // float endstop_strength_unit;
    // float snap_point;
    // char descriptor[50];

    {
        0,
        0,
        10 * PI / 180,
        0,
        1,
        1.1,
        "Unbounded\nNo detents",
    },
    {
        11,
        0,
        10 * PI / 180,
        0,
        1,
        1.1,
        "Bounded 0-10\nNo detents",
    },
    {
        73,
        0,
        10 * PI / 180,
        0,
        1,
        1.1,
        "Multi-rev\nNo detents",
    },
    {
        2,
        0,
        60 * PI / 180,
        1,
        1,
        0.55, // Note the snap point is slightly past the midpoint (0.5); compare to normal detents which use a snap point *past* the next value (i.e. > 1)
        "On/off\nStrong detent",
    },
    {
        1,
        0,
        60 * PI / 180,
        0.01,
        0.6,
        1.1,
        "Return-to-center",
    },
    {
        256,
        127,
        1 * PI / 180,
        0,
        1,
        1.1,
        "Fine values\nNo detents",
    },
    {
        256,
        127,
        1 * PI / 180,
        1,
        1,
        1.1,
        "Fine values\nWith detents",
    },
    {
        32,
        0,
        8.225806452 * PI / 180,
        2,
        1,
        1.1,
        "Coarse values\nStrong detents",
    },
    {
        32,
        0,
        8.225806452 * PI / 180,
        0.2,
        1,
        1.1,
        "Coarse values\nWeak detents",
    },
};

InterfaceTask::InterfaceTask(const uint8_t task_core, MotorTask& motor_task, DisplayTask* display_task) : Task("Interface", 4048, 1, task_core), motor_task_(motor_task), display_task_(display_task) {
    #if SK_DISPLAY
        assert(display_task != nullptr);
    #endif
}

InterfaceTask::~InterfaceTask() {}

void InterfaceTask::run() {
    #if PIN_BUTTON_NEXT >= 34
        pinMode(PIN_BUTTON_NEXT, INPUT);
    #else
        pinMode(PIN_BUTTON_NEXT, INPUT_PULLUP);
    #endif
    AceButton button_next((uint8_t) PIN_BUTTON_NEXT);
    button_next.getButtonConfig()->setIEventHandler(this);

    #if PIN_BUTTON_PREV > -1
        #if PIN_BUTTON_PREV >= 34
            pinMode(PIN_BUTTON_PREV, INPUT);
        #else
            pinMode(PIN_BUTTON_PREV, INPUT_PULLUP);
        #endif
        AceButton button_prev((uint8_t) PIN_BUTTON_PREV);
        button_prev.getButtonConfig()->setIEventHandler(this);
    #endif
    
    #if SK_LEDS
        FastLED.addLeds<SK6812, PIN_LED_DATA, GRB>(leds, NUM_LEDS);
    #endif

    #if PIN_SDA >= 0 && PIN_SCL >= 0
        Wire.begin(PIN_SDA, PIN_SCL);
        Wire.setClock(400000);
    #endif
    #if SK_STRAIN
        scale.begin(38, 2);
    #endif

    #if SK_ALS
        if (veml.begin()) {
            veml.setGain(VEML7700_GAIN_2);
            veml.setIntegrationTime(VEML7700_IT_400MS);
        } else {
            Serial.println("ALS sensor not found!");
        }
    #endif

    motor_task_.setConfig(configs[0]);

    // How far button is pressed, in range [0, 1]
    float press_value_unit = 0;

    // Interface loop:
    while (1) {
        button_next.check();
        #if PIN_BUTTON_PREV > -1
            button_prev.check();
        #endif
        if (Serial.available()) {
            int v = Serial.read();
            if (v == ' ') {
                changeConfig(true);
            }
        }

        #if SK_ALS
            const float LUX_ALPHA = 0.005;
            static float lux_avg;
            float lux = veml.readLux();
            lux_avg = lux * LUX_ALPHA + lux_avg * (1 - LUX_ALPHA);
            static uint32_t last_als;
            if (millis() - last_als > 1000) {
                Serial.print("millilux: "); Serial.println(lux*1000);
                last_als = millis();
            }
        #endif

        #if SK_STRAIN
            // TODO: calibrate and track (long term moving average) zero point (lower); allow calibration of set point offset
            const int32_t lower = 950000;
            const int32_t upper = 1800000;
            if (scale.wait_ready_timeout(100)) {
                int32_t reading = scale.read();

                // Ignore readings that are way out of expected bounds
                if (reading >= lower - (upper - lower) && reading < upper + (upper - lower)*2) {
                    static uint32_t last_reading_display;
                    if (millis() - last_reading_display > 1000) {
                        Serial.print("HX711 reading: ");
                        Serial.println(reading);
                        last_reading_display = millis();
                    }
                    long value = CLAMP(reading, lower, upper);
                    press_value_unit = 1. * (value - lower) / (upper - lower);

                    static bool pressed;
                    if (!pressed && press_value_unit > 0.75) {
                        motor_task_.playHaptic(true);
                        pressed = true;
                        changeConfig(true);
                    } else if (pressed && press_value_unit < 0.25) {
                        motor_task_.playHaptic(false);
                        pressed = false;
                    }
                }
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

        uint16_t brightness = UINT16_MAX;
        // TODO: brightness scale factor should be configurable (depends on reflectivity of surface)
        #if SK_ALS
            brightness = (uint16_t)CLAMP(lux_avg * 13000, (float)1280, (float)UINT16_MAX);
        #endif

        #if SK_DISPLAY
            display_task_->setBrightness(brightness); // TODO: apply gamma correction
        #endif

        #if SK_LEDS
            for (uint8_t i = 0; i < NUM_LEDS; i++) {
                leds[i].setHSV(200 * press_value_unit, 255, brightness >> 8);

                // Gamma adjustment
                leds[i].r = dim8_video(leds[i].r);
                leds[i].g = dim8_video(leds[i].g);
                leds[i].b = dim8_video(leds[i].b);
            }
            FastLED.show();
        #endif

        delay(10);
    }
}

void InterfaceTask::handleEvent(AceButton* button, uint8_t event_type, uint8_t button_state) {
    switch (event_type) {
        case AceButton::kEventPressed:
            if (button->getPin() == PIN_BUTTON_NEXT) {
                changeConfig(true);
            }
            #if PIN_BUTTON_PREV > -1
                if (button->getPin() == PIN_BUTTON_PREV) {
                    changeConfig(false);
                }
            #endif
            break;
        case AceButton::kEventReleased:
            break;
    }
}

void InterfaceTask::changeConfig(bool next) {
    if (next) {
        current_config_ = (current_config_ + 1) % COUNT_OF(configs);
    } else {
        if (current_config_ == 0) {
            current_config_ = COUNT_OF(configs) - 1;
        } else {
            current_config_ --;
        }
    }
    
    Serial.print("Changing config to ");
    Serial.print(current_config_);
    Serial.print(" -- ");
    Serial.println(configs[current_config_].descriptor);
    motor_task_.setConfig(configs[current_config_]);
}
