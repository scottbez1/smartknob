#pragma once

#include <Arduino.h>
#include <SimpleFOC.h>
#include <vector>

#include "knob_data.h"
#include "task.h"


enum class CommandType {
    CONFIG,
    HAPTIC,
};

struct HapticData {
    bool press;
};

struct Command {
    CommandType command_type;
    union CommandData {
        KnobConfig config;
        HapticData haptic;
    };
    CommandData data;
};

class MotorTask : public Task<MotorTask> {
    friend class Task<MotorTask>; // Allow base Task to invoke protected run()

    public:
        MotorTask(const uint8_t task_core);
        ~MotorTask();

        void setConfig(const KnobConfig& config);
        void playHaptic(bool press);

        void addListener(QueueHandle_t queue);

    protected:
        void run();

    private:
        QueueHandle_t queue_;

        std::vector<QueueHandle_t> listeners_;

        // BLDC motor & driver instance
        BLDCMotor motor = BLDCMotor(1);
        BLDCDriver6PWM driver = BLDCDriver6PWM(PIN_UH, PIN_UL, PIN_VH, PIN_VL, PIN_WH, PIN_WL);

        void publish(const KnobState& state);
        void calibrate();
};
