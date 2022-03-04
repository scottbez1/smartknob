#include "tlv_sensor.h"

static const float ALPHA = 0.04;

TlvSensor::TlvSensor() {}

void TlvSensor::init(TwoWire& wire, bool invert) {
  invert_ = invert;
  tlv_.begin(wire);
  tlv_.setAccessMode(Tlv493d::AccessMode_e::MASTERCONTROLLEDMODE);
  tlv_.disableInterrupt();
  tlv_.disableTemp();
}

float TlvSensor::getSensorAngle() {
    uint32_t now = micros();
    if (now - last_update_ > 100) {
      tlv_.updateData();
      x_ = tlv_.getX() * ALPHA + x_ * (1-ALPHA);
      y_ = tlv_.getY() * ALPHA + y_ * (1-ALPHA);
      last_update_ = now;
    }
    float rad = (invert_ ? -1 : 1) * atan2f(y_, x_);
    if (rad < 0) {
        rad += 2*PI;
    }
    return rad;
}
