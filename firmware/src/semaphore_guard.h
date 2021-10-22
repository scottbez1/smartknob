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

#include <Arduino.h>

class SemaphoreGuard {
    public:
        SemaphoreGuard(SemaphoreHandle_t handle) : handle_{handle} {
            xSemaphoreTake(handle_, portMAX_DELAY);
        }
        ~SemaphoreGuard() {
            xSemaphoreGive(handle_);
        }
        SemaphoreGuard(SemaphoreGuard const&)=delete;
        SemaphoreGuard& operator=(SemaphoreGuard const&)=delete;

    private:
        SemaphoreHandle_t handle_;
};
