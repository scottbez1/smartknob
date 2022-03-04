#pragma once

#include <Arduino.h>

#include "knob_data.h"
#include "task.h"

class MotorTask : public Task<MotorTask> {
    friend class Task<MotorTask>; // Allow base Task to invoke protected run()

    public:
        MotorTask(const uint8_t task_core);
        ~MotorTask();

        void setConfig(const KnobConfig& config);

        void addListener(QueueHandle_t queue);

    protected:
        void run();

    private:
        QueueHandle_t queue_;

        std::vector<QueueHandle_t> listeners_;

        void publish(const KnobState& state);
};
