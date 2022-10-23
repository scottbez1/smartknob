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

#include <driver/uart.h>

#include "config.h"
#include "uart_stream.h"

UartStream::UartStream() : Stream() {
}

void UartStream::begin() {
    uart_config_t conf;
    conf.baud_rate           = MONITOR_SPEED;
    conf.data_bits           = UART_DATA_8_BITS;
    conf.parity              = UART_PARITY_DISABLE;
    conf.stop_bits           = UART_STOP_BITS_1;
    conf.flow_ctrl           = UART_HW_FLOWCTRL_DISABLE;
    conf.rx_flow_ctrl_thresh = 0;
    conf.use_ref_tick        = false;
    assert(uart_param_config(uart_port_, &conf) == ESP_OK);
    assert(uart_driver_install(uart_port_, 32000, 32000, 0, NULL, 0) == ESP_OK);
}

int UartStream::peek() {
    return -1;
}

int UartStream::available() {
    size_t size = 0;
    assert(uart_get_buffered_data_len(uart_port_, &size) == ESP_OK);
    return size;
}

int UartStream::read() {
    uint8_t b;
    int res = uart_read_bytes(uart_port_, &b, 1, 0);
    return res != 1 ? -1 : b;
}

void UartStream::flush() {

}

size_t UartStream::write(uint8_t b) {
    return uart_write_bytes(uart_port_, (char*)&b, 1);
}

size_t UartStream::write(const uint8_t *buffer, size_t size) {
    return uart_write_bytes(uart_port_, (const char*)buffer, size);
}
