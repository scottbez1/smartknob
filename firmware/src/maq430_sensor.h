#pragma once
#include <sensors/MagneticSensorSPI.h>

/** Configured fro 12bit MA710 and MAQ430 magnetic sensor over SPI interface*/
MagneticSensorSPIConfig_s MAQ430_SPI = {
  .spi_mode = SPI_MODE3,
  .clock_speed = 1000000,
  .bit_resolution = 12,
  .angle_register = 0x0000,
  .data_start_bit = 15,
  .command_rw_bit = 0,  // not required
  .command_parity_bit = 17 // parity not implemented
};
