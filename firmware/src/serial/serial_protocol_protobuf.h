#pragma once

#include "../proto_gen/smartknob.pb.h"

#include "serial_protocol.h"
#include "uart_stream.h"

class SerialProtocolProtobuf : public SerialProtocol {
    public:
        SerialProtocolProtobuf(Stream& stream) : SerialProtocol(), stream_(stream) {}
        ~SerialProtocolProtobuf(){}
        void log(const char* msg) override;
        void loop() override;
        void handleState(const PB_SmartKnobState& state) override;

        void init();
    
    private:
        Stream& stream_;
};
