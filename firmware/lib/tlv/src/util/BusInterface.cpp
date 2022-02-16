/**
 * BusInterface.cpp - Part of the library for Arduino to control the TLV493D-A1B6 3D magnetic sensor.
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

#include "BusInterface2.h"

void tlv493d::initInterface(BusInterface_t *interface, TwoWire *bus, uint8_t adress)
{
	uint8_t i;
	interface->bus = bus;
	interface->adress = adress;
	for(i = 0; i < TLV493D_BUSIF_READSIZE; i++) {
		interface->regReadData[i] = 0x00;;
	}
	for(i = 0; i < TLV493D_BUSIF_WRITESIZE; i++) {
		interface->regWriteData[i] = 0x00;;
	}
}

bool tlv493d::readOut(BusInterface_t *interface)
{
	return readOut(interface, TLV493D_BUSIF_READSIZE);
}

bool tlv493d::readOut(BusInterface_t *interface, uint8_t count)
{
	bool ret = BUS_ERROR;
	int i;
	if(count > TLV493D_BUSIF_READSIZE)
	{
		count = TLV493D_BUSIF_READSIZE;
	}
	uint8_t received_bytes = interface->bus->requestFrom(interface->adress,count);
	if (received_bytes == count)
	{
		for(i = 0; i < count; i++)
		{
			interface->regReadData[i] = interface->bus->read();
		}
		ret = BUS_OK;
	}
	return ret;
}

bool tlv493d::writeOut(BusInterface_t *interface)
{
	return writeOut(interface, TLV493D_BUSIF_WRITESIZE);
}

bool tlv493d::writeOut(BusInterface_t *interface, uint8_t count)
{
	bool ret = BUS_ERROR;
	int i;
	if(count > TLV493D_BUSIF_WRITESIZE)
	{
		count = TLV493D_BUSIF_WRITESIZE;
	}
	interface->bus->beginTransmission(interface->adress);
	for(i = 0; i < count; i++)
	{
		interface->bus->write(interface->regWriteData[i]);
	}
	if (interface->bus->endTransmission() == 0)
	{
		ret = BUS_OK;
	}
	return ret;
}

