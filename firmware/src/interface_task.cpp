#include <AceButton.h>
#include "interface_task.h"

using namespace ace_button;

#define COUNT_OF(A) (sizeof(A) / sizeof(A[0]))

static KnobConfig configs[] = {
    // int32_t num_positions;
    // int32_t position;
    // float position_width_radians;
    // float detent_strength_unit;
    // float snap_point;
    // char descriptor[50];

    {
        0,
        0,
        10 * PI / 180,
        0,
        1.1,
        "Unbounded\nNo detents",
    },
    {
        11,
        0,
        10 * PI / 180,
        0,
        1.1,
        "Bounded 0-10\nNo detents",
    },
    {
        73,
        0,
        10 * PI / 180,
        0,
        1.1,
        "Multi-rev\nNo detents",
    },
    {
        2,
        0,
        45 * PI / 180,
        1,
        0.6, // Note the snap point is slightly past the midpoint (0.5); compare to normal detents which use a snap point *past* the next value (i.e. > 1)
        "On/off\nStrong detent",
    },
    {
        1,
        0,
        60 * PI / 180,
        0.01,
        1.1,
        "Return-to-center",
    },
    {
        256,
        127,
        1 * PI / 180,
        0,
        1.1,
        "Fine values\nNo detents",
    },
    {
        256,
        127,
        1 * PI / 180,
        1,
        1.1,
        "Fine values\nWith detents",
    },
    {
        32,
        0,
        8.225806452 * PI / 180,
        1,
        1.1,
        "Coarse values\nStrong detents",
    },
    {
        32,
        0,
        8.225806452 * PI / 180,
        0.1,
        1.1,
        "Coarse values\nWeak detents",
    },
};

InterfaceTask::InterfaceTask(const uint8_t task_core, MotorTask& motor_task) : Task{"Interface", 2048, 1, task_core}, motor_task_(motor_task) {
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
        delay(10);
    }
}

void InterfaceTask::handleEvent(AceButton* button, uint8_t event_type, uint8_t button_state) {
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
    Serial.printf("Changing config to %d:\n%s\n", current_config_, configs[current_config_].descriptor);
    motor_task_.setConfig(configs[current_config_]);
}
