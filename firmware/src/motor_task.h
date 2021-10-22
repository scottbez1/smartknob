#pragma once

#include <Arduino.h>

#include "task.h"
#include "display_task.h"

class MotorTask : public Task<MotorTask> {
    friend class Task<MotorTask>; // Allow base Task to invoke protected run()

    public:
        MotorTask(const uint8_t task_core, DisplayTask& display_task);
        ~MotorTask();

    protected:
        void run();

    private:
        DisplayTask& display_task_;
};
