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
#include "semaphore_guard.h"
#include "util.h"

#if SK_LEDS
CRGB leds[NUM_LEDS];
#endif

#if SK_STRAIN
HX711 scale;
#endif

#if SK_ALS
Adafruit_VEML7700 veml = Adafruit_VEML7700();
#endif

InterfaceTask::InterfaceTask(const uint8_t task_core, MotorTask& motor_task, DisplayTask* display_task) : 
        Task("Interface", 3400, 1, task_core),
        stream_(),
        motor_task_(motor_task),
        display_task_(display_task),
        plaintext_protocol_(stream_, [this] () {
            motor_task_.runCalibration();
        }),
        proto_protocol_(stream_, [this] (PB_SmartKnobConfig& config) {
            applyConfig(config, true);
        }) {
    #if SK_DISPLAY
        assert(display_task != nullptr);
    #endif


    log_queue_ = xQueueCreate(10, sizeof(std::string *));
    assert(log_queue_ != NULL);

    knob_state_queue_ = xQueueCreate(1, sizeof(PB_SmartKnobState));
    assert(knob_state_queue_ != NULL);

    mutex_ = xSemaphoreCreateMutex();
    assert(mutex_ != NULL);
}

InterfaceTask::~InterfaceTask() {
    vSemaphoreDelete(mutex_);
}

void InterfaceTask::run() {
    stream_.begin();
    
    #if SK_LEDS
        FastLED.addLeds<SK6812, PIN_LED_DATA, GRB>(leds, NUM_LEDS);
    #endif

    #if SK_ALS && PIN_SDA >= 0 && PIN_SCL >= 0
        Wire.begin(PIN_SDA, PIN_SCL);
        Wire.setClock(400000);
    #endif
    #if SK_STRAIN
        scale.begin(PIN_STRAIN_DO, PIN_STRAIN_SCK);
    #endif

    #if SK_ALS
        if (veml.begin()) {
            veml.setGain(VEML7700_GAIN_2);
            veml.setIntegrationTime(VEML7700_IT_400MS);
        } else {
            log("ALS sensor not found!");
        }
    #endif

    // app_position_ is sized from APP_SLOTS, but APP_COUNT is only known at link time.
    assert(APP_COUNT > 0 && APP_COUNT <= APP_SLOTS);

    motor_task_.addListener(knob_state_queue_);
    openMenu();

    plaintext_protocol_.init([this] () {
        handlePress();
    }, [this] () {
        handleBack();
    }, [this] () {
        if (!configuration_loaded_) {
            return;
        }
        if (strain_calibration_step_ == 0) {
            log("Strain calibration step 1: Don't touch the knob, then press 'S' again");
            strain_calibration_step_ = 1;
        } else if (strain_calibration_step_ == 1) {
            configuration_value_.strain.idle_value = strain_reading_;
            snprintf(buf_, sizeof(buf_), "  idle_value=%d", configuration_value_.strain.idle_value);
            log(buf_);
            log("Strain calibration step 2: Push and hold down the knob with medium pressure, and press 'S' again");
            strain_calibration_step_ = 2;
        } else if (strain_calibration_step_ == 2) {
            configuration_value_.strain.press_delta = strain_reading_ - configuration_value_.strain.idle_value;
            configuration_value_.has_strain = true;
            snprintf(buf_, sizeof(buf_), "  press_delta=%d", configuration_value_.strain.press_delta);
            log(buf_);
            log("Strain calibration complete! Saving...");
            strain_calibration_step_ = 0;
            if (configuration_->setStrainCalibrationAndSave(configuration_value_.strain)) {
                log("  Saved!");
            } else {
                log("  FAILED to save config!!!");
            }
        }
    });

    // Start in legacy protocol mode
    current_protocol_ = &plaintext_protocol_;

    ProtocolChangeCallback protocol_change_callback = [this] (uint8_t protocol) {
        switch (protocol) {
            case SERIAL_PROTOCOL_LEGACY:
                current_protocol_ = &plaintext_protocol_;
                break;
            case SERIAL_PROTOCOL_PROTO:
                current_protocol_ = &proto_protocol_;
                break;
            default:
                log("Unknown protocol requested");
                break;
        }
    };

    plaintext_protocol_.setProtocolChangeCallback(protocol_change_callback);
    proto_protocol_.setProtocolChangeCallback(protocol_change_callback);

    // Interface loop:
    while (1) {
        if (xQueueReceive(knob_state_queue_, &latest_state_, 0) == pdTRUE) {
            publishState();

            if (ui_state_.mode == UiMode::MENU) {
                menu_selection_ = (uint8_t)CLAMP(latest_state_.current_position, (int32_t)0, (int32_t)(APP_COUNT - 1));
            } else if (ui_state_.mode == UiMode::APP) {
                app_position_[ui_state_.app_index] = latest_state_.current_position;
            }
        }

        current_protocol_->loop();

        std::string* log_string;
        while (xQueueReceive(log_queue_, &log_string, 0) == pdTRUE) {
            current_protocol_->log(log_string->c_str());
            delete log_string;
        }

        updateHardware();

        if (!configuration_loaded_) {
            SemaphoreGuard lock(mutex_);
            if (configuration_ != nullptr) {
                configuration_value_ = configuration_->get();
                configuration_loaded_ = true;
            }
        }

        delay(1);
    }
}

void InterfaceTask::log(const char* msg) {
    // Allocate a string for the duration it's in the queue; it is free'd by the queue consumer
    std::string* msg_str = new std::string(msg);

    // Put string in queue (or drop if full to avoid blocking)
    xQueueSendToBack(log_queue_, &msg_str, 0);
}

void InterfaceTask::openMenu() {
    ui_state_.mode = UiMode::MENU;
    ui_state_.hold_progress = 0;
    ui_state_.mode_nonce++;
    publishUiState();

    PB_SmartKnobConfig config = menuConfig(menu_selection_);
    config.position_nonce = local_nonce_++;
    applyConfig(config, false);
}

void InterfaceTask::openApp(uint8_t index) {
    const AppDescriptor& app = APPS[index];

    bool entering = ui_state_.mode != UiMode::APP || ui_state_.app_index != index;

    ui_state_.mode = UiMode::APP;
    ui_state_.app_index = index;
    ui_state_.hold_progress = 0;
    if (entering) {
        // Only a real page change animates; in-app value changes must not restart it.
        ui_state_.mode_nonce++;
    }
    publishUiState();

    PB_SmartKnobConfig config = app.config;
    config.position = app_position_[index];
    config.sub_position_unit = 0;
    config.position_nonce = local_nonce_++;
    applyConfig(config, false);

    snprintf(buf_, sizeof(buf_), "Opening %s", app.name);
    log(buf_);
}

void InterfaceTask::handlePress() {
    if (remote_controlled_) {
        // The host owns the screen; it sees press_nonce and decides what to do.
        // Still ripple, so the tap gets acknowledged locally either way.
        ui_state_.press_nonce++;
        publishUiState();
        return;
    }

    if (ui_state_.mode == UiMode::MENU) {
        openApp(menu_selection_);
        return;
    }

    // Inside an app, a short press runs that app's own action.
    uint8_t index = ui_state_.app_index;
    const AppDescriptor& app = APPS[index];
    int32_t current = app_position_[index];
    bool changed = false;

    switch (app.press_action) {
        case PressAction::TOGGLE:
            app_position_[index] = current != app.config.min_position
                    ? app.config.min_position
                    : app.config.max_position;
            changed = true;
            break;
        case PressAction::CYCLE:
            app_position_[index] = current >= app.config.max_position
                    ? app.config.min_position
                    : current + 1;
            changed = true;
            break;
        case PressAction::MUTE:
            if (current != app.config.min_position) {
                muted_position_ = current;
                app_position_[index] = app.config.min_position;
            } else {
                app_position_[index] = muted_position_;
            }
            changed = true;
            break;
        case PressAction::NONE:
            break;
    }

    ui_state_.press_nonce++;
    if (changed) {
        // Re-send the config to snap the motor to the new position.
        openApp(index);
    } else {
        publishUiState();
    }
}

void InterfaceTask::handleBack() {
    if (ui_state_.mode == UiMode::MENU) {
        return;
    }
    remote_controlled_ = false;
    motor_task_.playHaptic(false);
    openMenu();
}

void InterfaceTask::publishUiState() {
    #if SK_DISPLAY
        display_task_->setUiState(ui_state_);
    #endif
}

void InterfaceTask::updateHardware() {
    // How far button is pressed, in range [0, 1]
    float press_value_unit = 0;

    #if SK_ALS
        const float LUX_ALPHA = 0.005;
        static float lux_avg;
        float lux = veml.readLux();
        lux_avg = lux * LUX_ALPHA + lux_avg * (1 - LUX_ALPHA);
        static uint32_t last_als;
        if (millis() - last_als > 1000 && strain_calibration_step_ == 0) {
            snprintf(buf_, sizeof(buf_), "millilux: %.2f", lux*1000);
            log(buf_);
            last_als = millis();
        }
    #endif

    #if SK_STRAIN
        if (scale.wait_ready_timeout(100)) {
            strain_reading_ = scale.read();

            static uint32_t last_reading_display;
            if (millis() - last_reading_display > 1000 && strain_calibration_step_ == 0) {
                snprintf(buf_, sizeof(buf_), "HX711 reading: %d", strain_reading_);
                log(buf_);
                last_reading_display = millis();
            }
            if (configuration_loaded_ && configuration_value_.has_strain && strain_calibration_step_ == 0) {
                // TODO: calibrate and track (long term moving average) idle point (lower)
                press_value_unit = lerp(strain_reading_, configuration_value_.strain.idle_value, configuration_value_.strain.idle_value + configuration_value_.strain.press_delta, 0, 1);

                // Ignore readings that are way out of expected bounds
                if (-1 < press_value_unit && press_value_unit < 2) {
                    static uint8_t press_readings;
                    if (!knob_pressed_ && press_value_unit > 1) {
                        press_readings++;
                        if (press_readings > 2) {
                            motor_task_.playHaptic(true);
                            knob_pressed_ = true;
                            press_started_ms_ = millis();
                            hold_consumed_ = false;
                            press_count_++;
                            publishState();
                        }
                    } else if (knob_pressed_ && press_value_unit < 0.5) {
                        press_readings++;
                        if (press_readings > 2) {
                            motor_task_.playHaptic(false);
                            knob_pressed_ = false;
                            // A hold already acted, so the release must not.
                            if (!hold_consumed_) {
                                handlePress();
                            }
                            ui_state_.hold_progress = 0;
                            publishUiState();
                        }
                    } else {
                        press_readings = 0;
                    }

                    if (knob_pressed_ && !hold_consumed_) {
                        float progress = (float)(millis() - press_started_ms_) / HOLD_TO_EXIT_MS;
                        bool can_exit = ui_state_.mode != UiMode::MENU;
                        ui_state_.hold_progress = can_exit ? CLAMP(progress, (float)0, (float)1) : 0;
                        if (can_exit && progress >= 1) {
                            hold_consumed_ = true;
                            ui_state_.hold_progress = 0;
                            handleBack();
                        } else {
                            publishUiState();
                        }
                    }
                }
            }
        } else {
            log("HX711 not found.");

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
            leds[i].setHSV(latest_config_.led_hue, 255 - 180*CLAMP(press_value_unit, (float)0, (float)1) - 75*knob_pressed_, brightness >> 8);

            // Gamma adjustment
            leds[i].r = dim8_video(leds[i].r);
            leds[i].g = dim8_video(leds[i].g);
            leds[i].b = dim8_video(leds[i].b);
        }
        FastLED.show();
    #endif
}

void InterfaceTask::setConfiguration(Configuration* configuration) {
    SemaphoreGuard lock(mutex_);
    configuration_ = configuration;
}

void InterfaceTask::publishState() {
    // Apply local state before publishing to serial
    latest_state_.press_nonce = press_count_;
    current_protocol_->handleState(latest_state_);
}

void InterfaceTask::applyConfig(PB_SmartKnobConfig& config, bool from_remote) {
    if (from_remote && ui_state_.mode != UiMode::REMOTE) {
        // A host took over: give it its own page instead of leaving the menu
        // carousel to follow a position that no longer selects an app.
        ui_state_.mode = UiMode::REMOTE;
        ui_state_.hold_progress = 0;
        ui_state_.mode_nonce++;
        publishUiState();
    }
    remote_controlled_ = from_remote;
    latest_config_ = config;
    motor_task_.setConfig(config);
}
