#include "../proto_gen/smartknob.pb.h"

#include "serial_protocol_plaintext.h"

void SerialProtocolPlaintext::handleState(const PB_SmartKnobState& state) {
    bool substantial_change = (latest_state_.current_position != state.current_position)
        || (latest_state_.config.detent_strength_unit != state.config.detent_strength_unit)
        || (latest_state_.config.endstop_strength_unit != state.config.endstop_strength_unit)
        || (latest_state_.config.num_positions != state.config.num_positions);
    latest_state_ = state;

    if (substantial_change) {
        stream_.printf("STATE: %d/%d  (detent strength: %0.2f, width: %0.0f deg, endstop strength: %0.2f)\n", state.current_position, state.config.num_positions - 1, state.config.detent_strength_unit, degrees(state.config.position_width_radians), state.config.endstop_strength_unit);
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
            if (demo_config_change_callback_) {
                demo_config_change_callback_();
            }
        } else if (b == 'C') {
            motor_task_.runCalibration();
        }
    }
}

void SerialProtocolPlaintext::init(DemoConfigChangeCallback cb) {
    demo_config_change_callback_ = cb;
    stream_.println("SmartKnob starting!\n\nSerial mode: plaintext\nPress 'C' at any time to calibrate.\nPress <Space> to change haptic modes.");
}
