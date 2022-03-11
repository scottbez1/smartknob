/**
 * Tlv493d_conf.h - Part of the library for Arduino to control the TLV493D-A1B6 3D magnetic sensor.
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

#ifndef TLV493D_CONF_H_INCLUDED
#define TLV493D_CONF_H_INCLUDED

#include "RegMask.h"
#include "Tlv493d.h"


#define TLV493D_DEFAULTMODE			POWERDOWNMODE

#define TLV493D_STARTUPDELAY		40
#define TLV493D_RESETDELAY			60

#define TLV493D_NUM_OF_REGMASKS		25
#define TLV493D_NUM_OF_ACCMODES		5

#define TLV493D_MEASUREMENT_READOUT	7
#define TLV493D_FAST_READOUT		3

#define TLV493D_B_MULT 				0.098
#define TLV493D_TEMP_MULT 			1.1
#define TLV493D_TEMP_OFFSET 		315


namespace tlv493d
{

typedef struct
{
	uint8_t fast;
	uint8_t lp;
	uint8_t lpPeriod;
	uint16_t measurementTime;
} AccessMode_t;

enum Registers_e
{
	R_BX1 = 0,
	R_BX2, 
	R_BY1, 
	R_BY2, 
	R_BZ1, 
	R_BZ2, 
	R_TEMP1, 
	R_TEMP2, 
	R_FRAMECOUNTER, 
	R_CHANNEL,
	R_POWERDOWNFLAG, 
	R_RES1,
	R_RES2,
	R_RES3,
	W_PARITY,
	W_ADDR,
	W_INT,
	W_FAST,
	W_LOWPOWER,
	W_TEMP_NEN,
	W_LP_PERIOD,
	W_PARITY_EN,
	W_RES1,
	W_RES2,
	W_RES3
};

const RegMask_t regMasks[] = {
	{ REGMASK_READ, 0, 0xFF, 0 },		// R_BX1
	{ REGMASK_READ, 4, 0xF0, 4 },		// R_BX2
	{ REGMASK_READ, 1, 0xFF, 0 },		// R_BY1
	{ REGMASK_READ, 4, 0x0F, 0 },		// R_BY2
	{ REGMASK_READ, 2, 0xFF, 0 },		// R_BZ1
	{ REGMASK_READ, 5, 0x0F, 0 },		// R_BZ2
	{ REGMASK_READ, 3, 0xF0, 4 },		// R_TEMP1
	{ REGMASK_READ, 6, 0xFF, 0 },		// R_TEMP2
	{ REGMASK_READ, 3, 0x0C, 2 },		// R_FRAMECOUNTER
	{ REGMASK_READ, 3, 0x03, 0 },		// R_CHANNEL
	{ REGMASK_READ, 5, 0x10, 4 },		// R_POWERDOWNFLAG
	{ REGMASK_READ, 7, 0x18, 3 },		// R_RES1
	{ REGMASK_READ, 8, 0xFF, 0 },		// R_RES2
	{ REGMASK_READ, 9, 0x1F, 0 },		// R_RES3
	{ REGMASK_WRITE, 1, 0x80, 7 },		// W_PARITY
	{ REGMASK_WRITE, 1, 0x60, 5 },		// W_ADDR
	{ REGMASK_WRITE, 1, 0x04, 2 },		// W_INT
	{ REGMASK_WRITE, 1, 0x02, 1 },		// W_FAST
	{ REGMASK_WRITE, 1, 0x01, 0 },		// W_LOWPOWER
	{ REGMASK_WRITE, 3, 0x80, 7 },		// W_TEMP_EN
	{ REGMASK_WRITE, 3, 0x40, 6 },		// W_LOWPOWER
	{ REGMASK_WRITE, 3, 0x20, 5 },		// W_POWERDOWN
	{ REGMASK_WRITE, 1, 0x18, 3 },		// W_RES1
	{ REGMASK_WRITE, 2, 0xFF, 0 },		// W_RES2
	{ REGMASK_WRITE, 3, 0x1F, 0 }		// W_RES3
};

const AccessMode_t accModes[] = {
	{ 0, 0, 0, 1000 },		// POWERDOWNMODE
	{ 1, 0, 0, 0 },			// FASTMODE
	{ 0, 1, 1, 10 },		// LOWPOWERMODE
	{ 0, 1, 0, 100 },		// ULTRALOWPOWERMODE
	{ 1, 1, 1, 10 }			// MASTERCONTROLLEDMODE
};

}

#endif
