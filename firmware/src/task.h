/*
   Copyright 2020 Scott Bezek and the splitflap contributors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#pragma once

#include<Arduino.h>

// Static polymorphic abstract base class for a FreeRTOS task using CRTP pattern. Concrete implementations
// should implement a run() method.
// Inspired by https://fjrg76.wordpress.com/2018/05/23/objectifying-task-creation-in-freertos-ii/
template<class T>
class Task {
    public:
        Task(const char* name, uint32_t stackDepth, UBaseType_t priority, const BaseType_t coreId = tskNO_AFFINITY) : 
                name { name },
                stackDepth {stackDepth},
                priority { priority },
                coreId { coreId }
        {}
        virtual ~Task() {};

        TaskHandle_t getHandle() {
            return taskHandle;
        }

        void begin() {
            BaseType_t result = xTaskCreatePinnedToCore(taskFunction, name, stackDepth, this, priority, &taskHandle, coreId);
            assert("Failed to create task" && result == pdPASS);
        }

    private:
        static void taskFunction(void* params) {
            T* t = static_cast<T*>(params);
            t->run();
        }

        const char* name;
        uint32_t stackDepth;
        UBaseType_t priority;
        TaskHandle_t taskHandle;
        const BaseType_t coreId;
};
