#pragma once

#include <Arduino.h>

#include "motor_task.h"
#include "task.h"

class InterfaceTask : public Task<InterfaceTask> {
    friend class Task<InterfaceTask>; // Allow base Task to invoke protected run()

    public:
        InterfaceTask(const uint8_t task_core, MotorTask& motor_task);
        ~InterfaceTask();

    protected:
        void run();

    private:
        MotorTask& motor_task_;
};
