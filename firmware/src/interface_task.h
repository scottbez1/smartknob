#pragma once

#include <AceButton.h>
#include <Arduino.h>

#include "configuration.h"
#include "display_task.h"
#include "logger.h"
#include "motor_task.h"
#include "serial/serial_protocol_plaintext.h"
#include "serial/serial_protocol_protobuf.h"
#include "serial/uart_stream.h"
#include "task.h"
#include "ui/apps.h"
#include "ui/ui_state.h"

#ifndef SK_FORCE_UART_STREAM
    #define SK_FORCE_UART_STREAM 0
#endif

class InterfaceTask : public Task<InterfaceTask>, public Logger {
    friend class Task<InterfaceTask>; // Allow base Task to invoke protected run()

    public:
        InterfaceTask(const uint8_t task_core, MotorTask& motor_task, DisplayTask* display_task);
        virtual ~InterfaceTask();

        void log(const char* msg) override;
        void setConfiguration(Configuration* configuration);

    protected:
        void run();

    private:
    #if defined(CONFIG_IDF_TARGET_ESP32S3) && !SK_FORCE_UART_STREAM
        HWCDC stream_;
    #else
        UartStream stream_;
    #endif
        MotorTask& motor_task_;
        DisplayTask* display_task_;
        char buf_[128];

        SemaphoreHandle_t mutex_;
        Configuration* configuration_ = nullptr; // protected by mutex_

        PB_PersistentConfiguration configuration_value_;
        bool configuration_loaded_ = false;

        uint8_t strain_calibration_step_ = 0;
        int32_t strain_reading_ = 0;

        SerialProtocol* current_protocol_ = nullptr;
        bool remote_controlled_ = false;
        uint8_t press_count_ = 1;

        /** How long the knob must be held on an app page to return to the menu. */
        static const uint32_t HOLD_TO_EXIT_MS = 600;

        UiState ui_state_ = {};
        uint8_t menu_selection_ = 0;
        /** Last value of each app, so reopening one resumes where it was left. */
        int32_t app_position_[APP_SLOTS] = {};
        /** Position remembered by a MUTE press, restored by the next press. */
        int32_t muted_position_ = 0;
        /** Nonce for locally-applied configs, so the motor honours our position. */
        uint8_t local_nonce_ = 1;

        bool knob_pressed_ = false;
        uint32_t press_started_ms_ = 0;
        /** Set once a hold has fired, to keep the release from also acting. */
        bool hold_consumed_ = false;

        PB_SmartKnobState latest_state_ = {};
        PB_SmartKnobConfig latest_config_ = {};

        QueueHandle_t log_queue_;
        QueueHandle_t knob_state_queue_;
        SerialProtocolPlaintext plaintext_protocol_;
        SerialProtocolProtobuf proto_protocol_;

        void openMenu();
        void openApp(uint8_t index);
        /** Short press: open the highlighted app, or run the app's press action. */
        void handlePress();
        /** Hold, or 'B' over serial: back out to the menu. */
        void handleBack();
        void publishUiState();
        void updateHardware();
        void publishState();
        void applyConfig(PB_SmartKnobConfig& config, bool from_remote);
};
