#pragma once

#include <functional>

#include "../logger.h"
#include "../proto_gen/smartknob.pb.h"

#define SERIAL_PROTOCOL_LEGACY 0
#define SERIAL_PROTOCOL_PROTO 1

typedef std::function<void(uint8_t)> ProtocolChangeCallback;

class SerialProtocol : public Logger {
    public:
        SerialProtocol() : Logger() {}
        virtual ~SerialProtocol(){}

        virtual void loop() = 0;

        virtual void handleState(const PB_SmartKnobState& state) = 0;

        virtual void setProtocolChangeCallback(ProtocolChangeCallback cb) {
            protocol_change_callback_ = cb;
        }
    
    protected:
        ProtocolChangeCallback protocol_change_callback_;
};
