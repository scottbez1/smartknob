#pragma once

#include "../proto_gen/smartknob.pb.h"

#include "serial_protocol.h"
#include "uart_stream.h"

class SerialProtocolPlaintext : public SerialProtocol {
    public:
        SerialProtocolPlaintext(Stream& stream) : SerialProtocol(), stream_(stream) {}
        ~SerialProtocolPlaintext(){}
        void log(const char* msg) override;
        void loop() override;
        void handleState(const PB_SmartKnobState& state) override;

        void init();
    
    private:
        Stream& stream_;
        PB_SmartKnobState latest_state_ = {};
};
