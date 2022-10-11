#pragma once

#include <SimpleFOC.h>
#include <Tlv493d.h>

class TlvSensor : public Sensor {
    public:
        TlvSensor();

        // initialize the sensor hardware
        void init(TwoWire* wire, bool invert);

        // Get current shaft angle from the sensor hardware, and 
        // return it as a float in radians, in the range 0 to 2PI.
        //  - This method is pure virtual and must be implemented in subclasses.
        //    Calling this method directly does not update the base-class internal fields.
        //    Use update() when calling from outside code.
        float getSensorAngle();

        bool getAndClearError();
    private:
        Tlv493d tlv_ = Tlv493d();
        float x_;
        float y_;
        uint32_t last_update_;
        TwoWire* wire_;
        bool invert_;

        bool error_ = false;

        uint8_t frame_counts_[3] = {};
        uint8_t cur_frame_count_index_ = 0;
};
