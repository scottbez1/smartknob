/**
 * Tlv493d.cpp - Library for Arduino to control the TLV493D-A1B6 3D magnetic sensor.
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


#include "Tlv493d.h"
#include "./util/RegMask.h"
#include "./util/BusInterface2.h"
#include <math.h>


Tlv493d::Tlv493d(void)
{
	mXdata = 0;
	mYdata = 0;
	mZdata = 0;
	mTempdata = 0;
	mExpectedFrameCount = 0x00;
}


Tlv493d::~Tlv493d(void)
{
	end();
}

void Tlv493d::begin(void)
{
	begin(Wire, TLV493D_ADDRESS1, true);
}

void Tlv493d::begin(TwoWire &bus)
{
	begin(bus, TLV493D_ADDRESS1, true);
}

void Tlv493d::begin(TwoWire &bus, Tlv493d_Address_t slaveAddress, bool reset)
{
	/**
	 * Workaround for kit2go vdd enable
	 */
	#ifdef TLV493D_A1B6_KIT2GO
	pinMode(LED2, OUTPUT);
	digitalWrite(LED2, HIGH);
	delay(50);
	#endif
	
	initInterface(&mInterface, &bus, slaveAddress);
	delay(TLV493D_STARTUPDELAY);

	mInterface.bus->begin();

	if(reset == true)
	{
		resetSensor(mInterface.adress);
	}

	// get all register data from sensor
	tlv493d::readOut(&mInterface);
	// copy factory settings to write registers
	setRegBits(tlv493d::W_RES1, getRegBits(tlv493d::R_RES1));
	setRegBits(tlv493d::W_RES1, getRegBits(tlv493d::R_RES1));
	setRegBits(tlv493d::W_RES1, getRegBits(tlv493d::R_RES1));
	// enable parity detection
	setRegBits(tlv493d::W_PARITY_EN, 1);
	// config sensor to lowpower mode
	// also contains parity calculation and writeout to sensor
	setAccessMode(TLV493D_DEFAULTMODE);
}


void Tlv493d::end(void)
{
	disableInterrupt();
	setAccessMode(POWERDOWNMODE);
}


bool Tlv493d::setAccessMode(AccessMode_e mode)
{
	bool ret = BUS_ERROR;
	const tlv493d::AccessMode_t *modeConfig = &(tlv493d::accModes[mode]);
	setRegBits(tlv493d::W_FAST, modeConfig->fast);
	setRegBits(tlv493d::W_LOWPOWER, modeConfig->lp);
	setRegBits(tlv493d::W_LP_PERIOD, modeConfig->lpPeriod);
	calcParity();
	ret = tlv493d::writeOut(&mInterface);
	if ( ret != BUS_ERROR )
	{
		mMode = mode;
	}
	return ret;
}


void Tlv493d::enableInterrupt(void)
{
	setRegBits(tlv493d::W_INT, 1);
	calcParity();
	tlv493d::writeOut(&mInterface);
}


void Tlv493d::disableInterrupt(void)
{
	setRegBits(tlv493d::W_INT, 0);
	calcParity();
	tlv493d::writeOut(&mInterface);
}

void Tlv493d::enableTemp(void)
{
	setRegBits(tlv493d::W_TEMP_NEN, 0);
	calcParity();
	tlv493d::writeOut(&mInterface);
}


void Tlv493d::disableTemp(void)
{
	setRegBits(tlv493d::W_TEMP_NEN, 1);
	calcParity();
	tlv493d::writeOut(&mInterface);
}


uint16_t Tlv493d::getMeasurementDelay(void)
{
	return tlv493d::accModes[mMode].measurementTime;
}


Tlv493d_Error_t Tlv493d::updateData(void)
{
	Tlv493d_Error_t ret = TLV493D_NO_ERROR;
	// in POWERDOWNMODE, sensor has to be switched on for one measurement
	uint8_t powerdown = 0;
	if(mMode == POWERDOWNMODE) 
	{
		if (setAccessMode(MASTERCONTROLLEDMODE) != BUS_OK)
		{
			ret = TLV493D_BUS_ERROR;
		}
		delay(getMeasurementDelay());
		powerdown = 1;
	}
	if(ret == TLV493D_NO_ERROR)
	{
#ifdef TLV493D_ACCELERATE_READOUT
		// just read the most important results in FASTMODE, if this behaviour is desired
		if(mMode == FASTMODE)
		{
			if (readOut(&mInterface, TLV493D_FAST_READOUT) != BUS_OK)
			{
				ret = TLV493D_BUS_ERROR;
			}
		}
		else
		{
			if (readOut(&mInterface, TLV493D_MEASUREMENT_READOUT) != BUS_OK)
			{
				ret = TLV493D_BUS_ERROR;
			}
		}
#else
		if (readOut(&mInterface, TLV493D_MEASUREMENT_READOUT) != BUS_OK)
		{
			ret = TLV493D_BUS_ERROR;
		}
#endif
		if (ret == TLV493D_NO_ERROR)
		{
			// construct results from registers
			mXdata = concatResults(getRegBits(tlv493d::R_BX1), getRegBits(tlv493d::R_BX2), true);
			mYdata = concatResults(getRegBits(tlv493d::R_BY1), getRegBits(tlv493d::R_BY2), true);
			mZdata = concatResults(getRegBits(tlv493d::R_BZ1), getRegBits(tlv493d::R_BZ2), true);
			mTempdata = concatResults(getRegBits(tlv493d::R_TEMP1), getRegBits(tlv493d::R_TEMP2), false);
			// switch sensor back to POWERDOWNMODE, if it was in POWERDOWNMODE before
			if(powerdown)
			{
				if (setAccessMode(POWERDOWNMODE) != BUS_OK)
				{
					ret = TLV493D_BUS_ERROR;
				}
			}
			if (ret == TLV493D_NO_ERROR)
			{
				// if the return value is 0, all results are from the same frame
				// otherwise some results may be outdated
				if(getRegBits(tlv493d::R_CHANNEL) != 0)
				{
					ret = TLV493D_FRAME_ERROR;
				}
// Todo: removed due to a lot of frame errors
//				// if received frame count does not match expected one (frame count from 0 to 3)
//				else if( getRegBits(tlv493d::R_FRAMECOUNTER) != (mExpectedFrameCount % 4) )
//				{
//					ret = TLV493D_FRAME_ERROR;
//				}
			}
		}
	}
	mExpectedFrameCount = getRegBits(tlv493d::R_FRAMECOUNTER) + 1;
	return ret;
}

// SBEZEK
uint8_t Tlv493d::getExpectedFrameCount(void) {
	return mExpectedFrameCount;
}

float Tlv493d::getX(void)
{
	return static_cast<float>(mXdata) * TLV493D_B_MULT;
}


float Tlv493d::getY(void)
{
	return static_cast<float>(mYdata) * TLV493D_B_MULT;
}


float Tlv493d::getZ(void)
{
	return static_cast<float>(mZdata) * TLV493D_B_MULT;
}


float Tlv493d::getTemp(void)
{
	return static_cast<float>(mTempdata-TLV493D_TEMP_OFFSET) * TLV493D_TEMP_MULT;
}


float Tlv493d::getAmount(void)
{
	// sqrt(x^2 + y^2 + z^2)
	return TLV493D_B_MULT * sqrt(pow(static_cast<float>(mXdata), 2) + pow(static_cast<float>(mYdata), 2) + pow(static_cast<float>(mZdata), 2));
}


float Tlv493d::getAzimuth(void)
{
	// arctan(y/x)
	return atan2(static_cast<float>(mYdata), static_cast<float>(mXdata));
}


float Tlv493d::getPolar(void)
{
	// arctan(z/(sqrt(x^2+y^2)))
	return atan2(static_cast<float>(mZdata), sqrt(pow(static_cast<float>(mXdata), 2) + pow(static_cast<float>(mYdata), 2)));
}


/* internal function called by begin()
 * The sensor has a special reset sequence which allows to change its i2c address by setting SDA to high or low during a reset. 
 * As some i2c peripherals may not cope with this, the simplest way is to use for this very few bytes bitbanging on the SCL/SDA lines.
 * Furthermore, as the uC may be stopped during a i2c transmission, a special recovery sequence allows to bring the bus back to
 * an operating state.
 */
void Tlv493d::resetSensor(uint8_t adr)     // Recovery & Reset - this can be handled by any uC as it uses bitbanging
{
	mInterface.bus->beginTransmission(0x00);

	if (adr == TLV493D_ADDRESS1) {
		// if the sensor shall be initialized with i2c address 0x1F
		mInterface.bus->write(0xFF);
	} else {
		// if the sensor shall be initialized with address 0x5E
		mInterface.bus->write((uint8_t)0x00);
	}

	mInterface.bus->endTransmission(true);
}

void Tlv493d::setRegBits(uint8_t regMaskIndex, uint8_t data)
{
	if(regMaskIndex < TLV493D_NUM_OF_REGMASKS)
	{
		tlv493d::setToRegs(&(tlv493d::regMasks[regMaskIndex]), mInterface.regWriteData, data);
	}
}

uint8_t Tlv493d::getRegBits(uint8_t regMaskIndex)
{
	if(regMaskIndex < TLV493D_NUM_OF_REGMASKS)
	{
		const tlv493d::RegMask_t *mask = &(tlv493d::regMasks[regMaskIndex]);
		if(mask->rw == REGMASK_READ)
		{
			return tlv493d::getFromRegs(mask, mInterface.regReadData);
		}
		else
		{
			return tlv493d::getFromRegs(mask, mInterface.regWriteData);
		}
	}
	return 0;
}

void Tlv493d::calcParity(void) 
{
	uint8_t i;
	uint8_t y = 0x00;
	// set parity bit to 1
	// algorithm will calculate an even parity and replace this bit, 
	// so parity becomes odd
	setRegBits(tlv493d::W_PARITY, 1);
	// combine array to one byte first
	for(i = 0; i < TLV493D_BUSIF_WRITESIZE; i++)
	{
		y ^= mInterface.regWriteData[i];
	}
	// combine all bits of this byte
	y = y ^ (y >> 1);
	y = y ^ (y >> 2);
	y = y ^ (y >> 4);
	// parity is in the LSB of y
	setRegBits(tlv493d::W_PARITY, y&0x01);
}


int16_t Tlv493d::concatResults(uint8_t upperByte, uint8_t lowerByte, bool upperFull)  
{
	int16_t value=0x0000;	//16-bit signed integer for 12-bit values of sensor
	if(upperFull)               
	{
		value=upperByte<<8;
		value|=(lowerByte&0x0F)<<4;
	}
	else
	{
		value=(upperByte&0x0F)<<12;
		value|=lowerByte<<4;
	}
	value>>=4;				//shift left so that value is a signed 12 bit integer
	return value;
}

