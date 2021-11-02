#include <AceButton.h>
#include "interface_task.h"

using namespace ace_button;

#define COUNT_OF(A) (sizeof(A) / sizeof(A[0]))

static KnobConfig configs[] = {
    {
        .num_positions = 0,
        .position = 0,
        .position_width_radians = 10 * PI / 180,
        .detent_strength_unit = 0,
        .snap_point = 1.1,
    },
    {
        .num_positions = 11,
        .position = 0,
        .position_width_radians = 10 * PI / 180,
        .detent_strength_unit = 0,
        .snap_point = 1.1,
    },
    {
        .num_positions = 73,
        .position = 0,
        .position_width_radians = 10 * PI / 180,
        .detent_strength_unit = 0,
        .snap_point = 1.1,
    },
    {
        .num_positions = 2,
        .position = 0,
        .position_width_radians = 60 * PI / 180,
        .detent_strength_unit = 1,
        .snap_point = 1.1,
    },
    {
        .num_positions = 2,
        .position = 0,
        .position_width_radians = 60 * PI / 180,
        .detent_strength_unit = 1,
        .snap_point = 0.6,
    },
    {
        .num_positions = 256,
        .position = 127,
        .position_width_radians = 1 * PI / 180,
        .detent_strength_unit = 0,
        .snap_point = 1.1,
    },
    {
        .num_positions = 256,
        .position = 127,
        .position_width_radians = 1 * PI / 180,
        .detent_strength_unit = 1,
        .snap_point = 1.1,
    },
    {
        .num_positions = 32,
        .position = 0,
        .position_width_radians = 8.225806452 * PI / 180,
        .detent_strength_unit = 1,
        .snap_point = 1.1,
    },
    {
        .num_positions = 32,
        .position = 0,
        .position_width_radians = 8.225806452 * PI / 180,
        .detent_strength_unit = 0.1,
        .snap_point = 1.1,
    },
};

InterfaceTask::InterfaceTask(const uint8_t task_core, MotorTask& motor_task) : Task{"Interface", 8192, 1, task_core}, motor_task_(motor_task) {
}

InterfaceTask::~InterfaceTask() {}

void InterfaceTask::run() {
    AceButton button(36);
    pinMode(36, INPUT);
    button.getButtonConfig()->setIEventHandler(this);

    motor_task_.setConfig(configs[0]);
    while (1) {
        button.check();
        if (Serial.available()) {
            int v = Serial.read();
            if (v == ' ') {
                nextConfig();
            }
        }
        // Serial.println(digitalRead(36));
        delay(10);
    }
}

void InterfaceTask::handleEvent(AceButton* button, uint8_t event_type, uint8_t button_state) {
    Serial.println("EVENT!");
    switch (event_type) {
        case AceButton::kEventPressed:
            nextConfig();
            break;
        case AceButton::kEventReleased:
            break;
    }
}

void InterfaceTask::nextConfig() {
    current_config_ = (current_config_ + 1) % COUNT_OF(configs);
    Serial.printf("Changing config to %d\n", current_config_);
    motor_task_.setConfig(configs[current_config_]);
}
