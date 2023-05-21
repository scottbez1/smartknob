
#include "MagneticSensorSPI.h"

/** Typical configuration for the 14bit AMS AS5147 magnetic sensor over SPI interface */
MagneticSensorSPIConfig_s AS5147_SPI = {
  .spi_mode = SPI_MODE1,
  .clock_speed = 1000000,
  .bit_resolution = 14,
  .angle_register = 0x3FFF,
  .data_start_bit = 13,
  .command_rw_bit = 14,
  .command_parity_bit = 15
};
// AS5048 and AS5047 are the same as AS5147
MagneticSensorSPIConfig_s AS5048_SPI = AS5147_SPI;
MagneticSensorSPIConfig_s AS5047_SPI = AS5147_SPI;

/** Typical configuration for the 14bit MonolithicPower MA730 magnetic sensor over SPI interface */
MagneticSensorSPIConfig_s MA730_SPI = {
  .spi_mode = SPI_MODE0,
  .clock_speed = 1000000,
  .bit_resolution = 14,
  .angle_register = 0x0000,
  .data_start_bit = 15,
  .command_rw_bit = 0,  // not required
  .command_parity_bit = 0 // parity not implemented
};

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


// MagneticSensorSPI(int cs, float _bit_resolution, int _angle_register)
//  cs              - SPI chip select pin
//  _bit_resolution   sensor resolution bit number
// _angle_register  - (optional) angle read register - default 0x3FFF
MagneticSensorSPI::MagneticSensorSPI(int cs, float _bit_resolution, int _angle_register){

  chip_select_pin = cs;
  // angle read register of the magnetic sensor
  angle_register = _angle_register ? _angle_register : DEF_ANGLE_REGISTER;
  // register maximum value (counts per revolution)
  cpr = pow(2,_bit_resolution);
  spi_mode = SPI_MODE1;
  clock_speed = 1000000;
  bit_resolution = _bit_resolution;

  command_parity_bit = 15; // for backwards compatibilty
  command_rw_bit = 14; // for backwards compatibilty
  data_start_bit = 13; // for backwards compatibilty
}

MagneticSensorSPI::MagneticSensorSPI(MagneticSensorSPIConfig_s config, int cs){
  chip_select_pin = cs;
  // angle read register of the magnetic sensor
  angle_register = config.angle_register ? config.angle_register : DEF_ANGLE_REGISTER;
  // register maximum value (counts per revolution)
  cpr = pow(2, config.bit_resolution);
  spi_mode = config.spi_mode;
  clock_speed = config.clock_speed;
  bit_resolution = config.bit_resolution;

  command_parity_bit = config.command_parity_bit; // for backwards compatibilty
  command_rw_bit = config.command_rw_bit; // for backwards compatibilty
  data_start_bit = config.data_start_bit; // for backwards compatibilty
}

void MagneticSensorSPI::init(SPIClass* _spi){
  spi = _spi;
	// 1MHz clock (AMS should be able to accept up to 10MHz)
	settings = SPISettings(clock_speed, MSBFIRST, spi_mode);
	//setup pins
	pinMode(chip_select_pin, OUTPUT);

	// SPI has an internal SPI-device counter, it is possible to call "begin()" from different devices
  // Manually Modified for NanoFOC
  // TODO: Add hspi class re-definition to main.cpp
	spi->begin(6, 7, 5, 4);
	// do any architectures need to set the clock divider for SPI? Why was this in the code?
  //spi->setClockDivider(SPI_CLOCK_DIV8);
	digitalWrite(chip_select_pin, HIGH);

  this->Sensor::init(); // call base class init
}

//  Shaft angle calculation
//  angle is in radians [rad]
float MagneticSensorSPI::getSensorAngle(){
  return -(getRawCount() / (float)cpr) * _2PI;
}

// function reading the raw counter of the magnetic sensor
int MagneticSensorSPI::getRawCount(){
	return (int)MagneticSensorSPI::read(angle_register);
}

// SPI functions 
/**
 * Utility function used to calculate even parity of word
 */
byte MagneticSensorSPI::spiCalcEvenParity(word value){
	byte cnt = 0;
	byte i;

	for (i = 0; i < 16; i++)
	{
		if (value & 0x1) cnt++;
		value >>= 1;
	}
	return cnt & 0x1;
}

  /*
  * Read a register from the sensor
  * Takes the address of the register as a 16 bit word
  * Returns the value of the register
  */
word MagneticSensorSPI::read(word angle_register){

  word command = angle_register;

  if (command_rw_bit > 0) {
    command = angle_register | (1 << command_rw_bit);
  }
  if (command_parity_bit > 0) {
   	//Add a parity bit on the the MSB
  	command |= ((word)spiCalcEvenParity(command) << command_parity_bit);
  }

  //SPI - begin transaction
  spi->beginTransaction(settings);

  //Send the command
  digitalWrite(chip_select_pin, LOW);
  spi->transfer16(command);
  digitalWrite(chip_select_pin,HIGH);
  
#if defined(ESP_H) && defined(ARDUINO_ARCH_ESP32) // if ESP32 board
  delayMicroseconds(50); // why do we need to delay 50us on ESP32? In my experience no extra delays are needed, on any of the architectures I've tested...
#else
  delayMicroseconds(1); // delay 1us, the minimum time possible in plain arduino. 350ns is the required time for AMS sensors, 80ns for MA730, MA702
#endif

  //Now read the response
  digitalWrite(chip_select_pin, LOW);
  word register_value = spi->transfer16(0x00);
  digitalWrite(chip_select_pin, HIGH);

  //SPI - end transaction
  spi->endTransaction();

  register_value = register_value >> (1 + data_start_bit - bit_resolution);  //this should shift data to the rightmost bits of the word

  const static word data_mask = 0xFFFF >> (16 - bit_resolution);

	return register_value & data_mask;  // Return the data, stripping the non data (e.g parity) bits
}

/**
 * Closes the SPI connection
 * SPI has an internal SPI-device counter, for each init()-call the close() function must be called exactly 1 time
 */
void MagneticSensorSPI::close(){
	spi->end();
}


