#pragma once

#include <Arduino.h>
#include <SimpleFOC.h>
#include <vector>

#include "knob_data.h"
#include "logger.h"
#include "task.h"


enum class CommandType {
    CALIBRATE,
    CONFIG,
    HAPTIC,
};

struct HapticData {
    bool press;
};

struct Command {
    CommandType command_type;
    union CommandData {
        uint8_t unused;
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
        void runCalibration();

        void addListener(QueueHandle_t queue);
        void setLogger(Logger* logger);

    protected:
        void run();

    private:
        QueueHandle_t queue_;
        Logger* logger_;
        std::vector<QueueHandle_t> listeners_;
        char buf_[72];

        // BLDC motor & driver instance
        BLDCMotor motor = BLDCMotor(1);
        BLDCDriver6PWM driver = BLDCDriver6PWM(PIN_UH, PIN_UL, PIN_VH, PIN_VL, PIN_WH, PIN_WL);

        void publish(const KnobState& state);
        void calibrate();
        void checkSensorError();
        void log(const char* msg);
};
