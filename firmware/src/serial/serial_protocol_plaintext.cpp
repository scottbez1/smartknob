#include "../proto_gen/smartknob.pb.h"

#include "serial_protocol_plaintext.h"

void SerialProtocolPlaintext::handleState(const PB_SmartKnobState& state) {
    bool substantial_change = (latest_state_.current_position != state.current_position)
        || (latest_state_.config.detent_strength_unit != state.config.detent_strength_unit)
        || (latest_state_.config.endstop_strength_unit != state.config.endstop_strength_unit)
        || (latest_state_.config.min_position != state.config.min_position)
        || (latest_state_.config.max_position != state.config.max_position);
    latest_state_ = state;

    if (substantial_change) {
        stream_.printf("STATE: %d [%d, %d]  (detent strength: %0.2f, width: %0.0f deg, endstop strength: %0.2f)\n", 
            state.current_position,
            state.config.min_position,
            state.config.max_position,
            state.config.detent_strength_unit,
            degrees(state.config.position_width_radians),
            state.config.endstop_strength_unit);
    }
}

void SerialProtocolPlaintext::log(const char* msg) {
    stream_.print("LOG: ");
    stream_.println(msg);
}

void SerialProtocolPlaintext::loop() {
    while (stream_.available() > 0) {
        int b = stream_.read();
        if (b == 0) {
            if (protocol_change_callback_) {
                protocol_change_callback_(SERIAL_PROTOCOL_PROTO);
            }
            break;
        }
        if (b == ' ') {
            if (press_callback_) {
                press_callback_();
            }
        } else if (b == 'B' || b == 'b') {
            if (back_callback_) {
                back_callback_();
            }
        } else if (b == 'C') {
            motor_calibration_callback_();
        } else if (b == 'S') {
            if (strain_calibration_callback_) {
                strain_calibration_callback_();
            }
        }
    }
}

void SerialProtocolPlaintext::init(PressCallback press_callback, BackCallback back_callback, StrainCalibrationCallback strain_calibration_callback) {
    press_callback_ = press_callback;
    back_callback_ = back_callback;
    strain_calibration_callback_ = strain_calibration_callback;
    stream_.println("SmartKnob starting!\n\nSerial mode: plaintext\nPress 'C' at any time to calibrate motor/sensor.\nPress 'S' at any time to calibrate strain sensors.\nPress <Space> to select (same as pressing the knob).\nPress 'B' to go back to the menu (same as holding the knob).");
}
