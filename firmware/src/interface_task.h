#pragma once

#include <AceButton.h>
#include <Arduino.h>

#include "display_task.h"
#include "logger.h"
#include "motor_task.h"
#include "serial/uart_stream.h"
#include "task.h"

class InterfaceTask : public Task<InterfaceTask>, public Logger, public ace_button::IEventHandler {
    friend class Task<InterfaceTask>; // Allow base Task to invoke protected run()

    public:
        InterfaceTask(const uint8_t task_core, MotorTask& motor_task, DisplayTask* display_task);
        ~InterfaceTask();

        void handleEvent(ace_button::AceButton* button, uint8_t event_type, uint8_t button_state) override;
        void log(const char* msg) override;

    protected:
        void run();

    private:
        UartStream stream_;
        MotorTask& motor_task_;
        DisplayTask* display_task_;
        char buf_[64];

        int current_config_ = 0;

        void changeConfig(bool next);
};
