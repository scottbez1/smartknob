#pragma once

#include <SimpleFOC.h>
#include "driver/spi_master.h"

class MT6701Sensor : public Sensor {
    public:
        MT6701Sensor();

        // initialize the sensor hardware
        void init();

        // Get current shaft angle from the sensor hardware, and 
        // return it as a float in radians, in the range 0 to 2PI.
        //  - This method is pure virtual and must be implemented in subclasses.
        //    Calling this method directly does not update the base-class internal fields.
        //    Use update() when calling from outside code.
        float getSensorAngle();
    private:

        spi_device_handle_t spi_device_;
        spi_transaction_t spi_transaction_ = {};

        float x_;
        float y_;
        uint32_t last_update_;
};
