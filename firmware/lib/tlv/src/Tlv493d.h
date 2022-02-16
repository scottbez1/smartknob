/**
 * Tlv493d.h - Library for Arduino to control the TLV493D-A1B6 3D magnetic sensor.
 *
 * The 3D magnetic sensor TLV493D-A1B6 offers accurate three dimensional sensing with extremely low power consumption 
 * in a small 6-pin package. With an opportunity to detect the magnetic field in x, y, and z-direction the sensor is 
 * ideally suited for the measurement of 3D movements, linear movements and rotation movements.
 * 
 * Have a look at the application note/reference manual for more information.
 * 
 * Copyright (c) 2018 Infineon Technologies AG
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
 * following conditions are met:   
 *                                                                              
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer.                        
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
 * disclaimer in the documentation and/or other materials provided with the distribution.                       
 * 
 * Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote 
 * products derived from this software without specific prior written permission.                                           
 *                                                                              
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE  
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR  
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   
 */

#ifndef TLV493D_H_INCLUDED
#define TLV493D_H_INCLUDED


#include <Arduino.h>
#include <Wire.h>
#include "./util/BusInterface.h"
#include "./util/Tlv493d_conf.h"

typedef enum Tlv493d_Address
{
	TLV493D_ADDRESS1	=	0x5E,
	TLV493D_ADDRESS2	=	0x1F
}Tlv493d_Address_t;


typedef enum Tlv493d_Error
{
	TLV493D_NO_ERROR	=	0,
	TLV493D_BUS_ERROR	=	1,
	TLV493D_FRAME_ERROR	=	2
}Tlv493d_Error_t;


/*
 * TLV493D_ACCELERATE_READOUT lets the controller just read out the first 3 bytes when in fast mode. 
 * This makes the readout faster (half of usual transfer duration), but there is no way to get 
 * temperature, current channel or high precision (only 8 instead of 12 bits for x, y, z)
 * It is necessary for slow I2C busses to read the last result before the new measurement is completed. 
 * It only takes effect in FASTMODE, not in other modes. 
 *
 * Feel free to undefine this and increase your I2C bus speed if you need to. 
 */
// SBEZEK
// #define TLV493D_ACCELERATE_READOUT


class Tlv493d
{
public: 

	Tlv493d(void);
	~Tlv493d(void);
	void begin(void);
	void begin(TwoWire &bus);
	void begin(TwoWire &bus, Tlv493d_Address_t slaveAddress, bool reset);
	void end(void);
	
	// sensor configuration
	/* sets the data access mode for TLE493D
	 * Tle493d is initially in POWERDOWNMODE
	 * use POWERDOWNMODE for rare and infrequent measurements 
	 * 	Tle493d will automatically switch to MASTERCONTROLLEDMODE for one measurement if on a readout
	 *	measurements are quite slow in this mode. The power consumption is very low between measurements. 
	 * use MASTERCONTROLLEDMODE for low measurement frequencies where results do not have to be up-to-date
	 *	In this mode a new measurement starts directly after the last result has been read out. 
	 * use LOWPOWERMODE and ULTRALOWPOWERMODE for continuous measurements
	 *	each readout returns the latest measurement results
	 * use FASTMODE for for continuous measurements on high frequencies
	 *	measurement time might be higher than the time necessary for I2C-readouts in this mode. 
	 *	Note: Thus, this mode requires a non-standard 1MHz I2C clock to be used to read the data fast enough.
	 */
	enum AccessMode_e
	{
		POWERDOWNMODE = 0,
		FASTMODE,
		LOWPOWERMODE,
		ULTRALOWPOWERMODE,
		MASTERCONTROLLEDMODE,
	};
	bool setAccessMode(AccessMode_e mode);
	// interrupt is disabled by default
	// it is recommended for FASTMODE, LOWPOWERMODE and ULTRALOWPOWERMODE
	// the interrupt is indicated with a short(1.5 us) low pulse on SCL
	// you need to capture and react(read the new results) to it by yourself
	void enableInterrupt(void);
	void disableInterrupt(void);
	// temperature measurement is enabled by default
	// it can be disabled to reduce power consumption
	void enableTemp(void);
	void disableTemp(void);
	
	// returns the recommended time between two readouts for the sensor's current configuration
	uint16_t getMeasurementDelay(void);
	// read measurement results from sensor
	Tlv493d_Error_t updateData(void);
	
	// fieldvector in Cartesian coordinates
	float getX(void);
	float getY(void);
	float getZ(void);
	
	// fieldvector in spherical coordinates
	float getAmount(void);
	float getAzimuth(void);
	float getPolar(void);
	
	// temperature
	float getTemp(void);

	// SBEZEK
	uint8_t getExpectedFrameCount(void);
	
private: 
	tlv493d::BusInterface_t mInterface;
	AccessMode_e mMode;
	int16_t mXdata;
	int16_t mYdata;
	int16_t mZdata;
	int16_t	mTempdata;
	uint8_t mExpectedFrameCount;
	

	void resetSensor(uint8_t adr);
	void setRegBits(uint8_t regMaskIndex, uint8_t data);
	uint8_t getRegBits(uint8_t regMaskIndex);
	void calcParity(void);
	int16_t concatResults(uint8_t upperByte, uint8_t lowerByte, bool upperFull);
};

#endif		/* TLV493D_H_INCLUDED */
