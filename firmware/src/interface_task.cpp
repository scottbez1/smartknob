#include "interface_task.h"

#define COUNT_OF(A) (sizeof(A) / sizeof(A[0]))

InterfaceTask::InterfaceTask(const uint8_t task_core, MotorTask& motor_task) : Task{"Interface", 8192, 1, task_core}, motor_task_(motor_task) {
}

InterfaceTask::~InterfaceTask() {}

void InterfaceTask::run() {
    KnobConfig configs[] = {
        {
            .num_positions = 0,
            .position = 0,
            .position_width_radians = 10 * PI / 180,
            .detent_strength_unit = 0,
        },
        {
            .num_positions = 11,
            .position = 0,
            .position_width_radians = 10 * PI / 180,
            .detent_strength_unit = 0,
        },
        {
            .num_positions = 2,
            .position = 0,
            .position_width_radians = 60 * PI / 180,
            .detent_strength_unit = 1,
        },
        {
            .num_positions = 256,
            .position = 127,
            .position_width_radians = 1 * PI / 180,
            .detent_strength_unit = 0,
        },
        {
            .num_positions = 256,
            .position = 127,
            .position_width_radians = 1 * PI / 180,
            .detent_strength_unit = 1,
        },
        {
            .num_positions = 32,
            .position = 0,
            .position_width_radians = 8.225806452 * PI / 180,
            .detent_strength_unit = 1,
        },
        {
            .num_positions = 32,
            .position = 0,
            .position_width_radians = 8.225806452 * PI / 180,
            .detent_strength_unit = 0.1,
        },
    };

    int current_config = 0;

    motor_task_.setConfig(configs[current_config]);
    while (1) {
        if (Serial.available()) {
            int v = Serial.read();
            if (v == ' ') {
                current_config = (current_config + 1) % COUNT_OF(configs);
                Serial.printf("Chaning config to %d\n", current_config);
                motor_task_.setConfig(configs[current_config]);
            }
        }
        delay(10);
    }
}
