#pragma once

#include "../proto_gen/smartknob.pb.h"

#include "motor_task.h"
#include "serial_protocol.h"
#include "uart_stream.h"

typedef std::function<void(void)> DemoConfigChangeCallback;

class SerialProtocolPlaintext : public SerialProtocol {
    public:
        SerialProtocolPlaintext(Stream& stream, MotorTask& motor_task) : SerialProtocol(), stream_(stream), motor_task_(motor_task) {}
        ~SerialProtocolPlaintext(){}
        void log(const char* msg) override;
        void loop() override;
        void handleState(const PB_SmartKnobState& state) override;

        void init(DemoConfigChangeCallback cb);
    
    private:
        Stream& stream_;
        MotorTask& motor_task_;
        PB_SmartKnobState latest_state_ = {};
        DemoConfigChangeCallback demo_config_change_callback_;
};
