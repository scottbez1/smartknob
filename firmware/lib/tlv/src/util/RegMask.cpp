/**
 * RegMask.cpp - Part of the library for Arduino to control the TLV493D-A1B6 3D magnetic sensor.
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

#include "RegMask.h"

uint8_t tlv493d::getFromRegs(const RegMask_t *mask, uint8_t *regData)
{
	return (regData[mask->byteAdress] & mask->bitMask) >> mask->shift;
}


uint8_t tlv493d::setToRegs(const RegMask_t *mask, uint8_t *regData, uint8_t toWrite)
{
	if(mask->rw == REGMASK_WRITE)
	{
		uint8_t regValue = regData[mask->byteAdress];
		regValue &= ~(mask->bitMask);
		regValue |= (toWrite << mask->shift) & mask->bitMask;
		regData[mask->byteAdress] = regValue;
	}
	return 0;
}



