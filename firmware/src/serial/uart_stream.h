/*
   Copyright 2021 Scott Bezek and the splitflap contributors

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

#include <driver/uart.h>

/**
 * Implementation of an Arduino Stream for UART serial communications using the esp uart driver
 * directly, rather than the Arduino HAL which has a small fixed underlying rx FIFO size and
 * potentially other issues that cause dropped bytes at high speeds/bursts.
 * 
 * This is not a full or optimized implementation; just the minimal necessary for this project.
 */
class UartStream : public Stream {
    public:
        UartStream();

        void begin();

        // Stream methods
        int available() override;
        int read() override;
        int peek() override;
        void flush() override;

        // Print methods
        size_t write(uint8_t b) override;
        size_t write(const uint8_t *buffer, size_t size) override;

    private:
        const uart_port_t uart_port_ = UART_NUM_0;
};
