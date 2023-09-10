#pragma once

#include "../proto_gen/smartknob.pb.h"

#include "interface_callbacks.h"
#include "motor_task.h"
#include "serial_protocol.h"
#include "uart_stream.h"

typedef std::function<void(void)> DemoConfigChangeCallback;
typedef std::function<void(void)> StrainCalibrationCallback;

class SerialProtocolPlaintext : public SerialProtocol {
    public:
        SerialProtocolPlaintext(Stream& stream, MotorCalibrationCallback motor_calibration_callback) : SerialProtocol(), stream_(stream), motor_calibration_callback_(motor_calibration_callback) {}
        ~SerialProtocolPlaintext(){}
        void log(const char* msg) override;
        void loop() override;
        void handleState(const PB_SmartKnobState& state) override;

        void init(DemoConfigChangeCallback demo_config_change_callback, StrainCalibrationCallback strain_calibration_callback);
    
    private:
        Stream& stream_;
        MotorCalibrationCallback motor_calibration_callback_;
        PB_SmartKnobState latest_state_ = {};
        DemoConfigChangeCallback demo_config_change_callback_;
        StrainCalibrationCallback strain_calibration_callback_;
};
