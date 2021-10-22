#include "tlv_sensor.h"

static const float ALPHA = 0.1;

TlvSensor::TlvSensor() {}

void TlvSensor::init() {
  tlv_.begin();
  tlv_.setAccessMode(Tlv493d::AccessMode_e::MASTERCONTROLLEDMODE);
  tlv_.disableInterrupt();
  tlv_.disableTemp();
}

float TlvSensor::getSensorAngle() {
    tlv_.updateData();
    x_ = tlv_.getX() * ALPHA + x_ * (1-ALPHA);
    y_ = tlv_.getY() * ALPHA + y_ * (1-ALPHA);
    float rad = atan2f(y_, x_);
    if (rad < 0) {
        rad += 2*PI;
    }
    return rad;
}
