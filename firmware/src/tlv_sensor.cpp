#include "tlv_sensor.h"

static const float ALPHA = 1;

TlvSensor::TlvSensor() {}

void TlvSensor::init(TwoWire* wire, bool invert) {
  wire_ = wire;
  invert_ = invert;
  tlv_.begin(*wire);
  tlv_.setAccessMode(Tlv493d::AccessMode_e::MASTERCONTROLLEDMODE);
  tlv_.disableInterrupt();
  tlv_.disableTemp();
}

float TlvSensor::getSensorAngle() {
    uint32_t now = micros();
    if (now - last_update_ > 50) {
      tlv_.updateData();
      frame_counts_[cur_frame_count_index_] = tlv_.getExpectedFrameCount();
      cur_frame_count_index_++;
      if (cur_frame_count_index_ >= sizeof(frame_counts_)) {
        cur_frame_count_index_ = 0;
      }
      x_ = tlv_.getX() * ALPHA + x_ * (1-ALPHA);
      y_ = tlv_.getY() * ALPHA + y_ * (1-ALPHA);
      last_update_ = now;

      bool all_same = true;
      uint8_t match_frame = frame_counts_[0];
      for (uint8_t i = 1; i < sizeof(frame_counts_); i++) {
        if (frame_counts_[i] != match_frame) {
          all_same = false;
          break;
        }
      }
      if (all_same) {
        error_ = true;
        init(wire_, invert_);
        // Force unique frame counts to avoid reset loop
        for (uint8_t i = 1; i < sizeof(frame_counts_); i++) {
          frame_counts_[i] = i;
        }
      }
    }
    float rad = (invert_ ? -1 : 1) * atan2f(y_, x_);
    if (rad < 0) {
        rad += 2*PI;
    }
    return rad;
}

bool TlvSensor::getAndClearError() {
  bool error = error_;
  error_ = false;
  return error;
}
