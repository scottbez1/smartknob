#pragma once

#include <Arduino.h>
#include <SimpleFOC.h>
#include <vector>

#include "configuration.h"
#include "logger.h"
#include "proto_gen/smartknob.pb.h"
#include "task.h"


enum class CommandType {
    CALIBRATE,
    CONFIG,
    HAPTIC,
};

struct HapticData {
    bool press;
    /**
     * Torque of the click. Clamped by the motor's voltage limit, so raising this
     * past FOC_VOLTAGE_LIMIT has no effect. 0 uses the press/release default.
     */
    float strength;
    /** How long each half of the click is driven, in ms. 0 uses the default. */
    uint8_t duration_ms;
};

struct Command {
    CommandType command_type;
    union CommandData {
        uint8_t unused;
        PB_SmartKnobConfig config;
        HapticData haptic;
    };
    CommandData data;
};

class MotorTask : public Task<MotorTask> {
    friend class Task<MotorTask>; // Allow base Task to invoke protected run()

    public:
        MotorTask(const uint8_t task_core, Configuration& configuration);
        ~MotorTask();

        void setConfig(const PB_SmartKnobConfig& config);
        /** Plays a click. Pass strength/duration_ms to override the defaults. */
        void playHaptic(bool press, float strength = 0, uint8_t duration_ms = 0);
        void runCalibration();

        void addListener(QueueHandle_t queue);
        void setLogger(Logger* logger);

    protected:
        void run();

    private:
        Configuration& configuration_;
        QueueHandle_t queue_;
        Logger* logger_;
        std::vector<QueueHandle_t> listeners_;
        char buf_[72];

        // BLDC motor & driver instance
        BLDCMotor motor = BLDCMotor(1);
        BLDCDriver6PWM driver = BLDCDriver6PWM(PIN_UH, PIN_UL, PIN_VH, PIN_VL, PIN_WH, PIN_WL);

        void publish(const PB_SmartKnobState& state);
        void calibrate();
        void checkSensorError();
        void log(const char* msg);
};
