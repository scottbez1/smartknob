# SmartKnob

Coming soon...

# Hardware

## Designs

### SmartKnob View
Premium SmartKnob experience. Under active development.

![Exploded view](doc/img/explodedv145.gif)

### SmartKnob Handheld
An exploration of a small handheld form-factor. Under active development.

### SmartKnob Mini
Planned for the future.

## Component Info

### Magnetic encoders
#### TLV493 (Infineon)
A decent choice, and easy to prototype with using [Adafruit's QWIIC breakout board](https://www.adafruit.com/product/4366).

In my testing, it is a little noisy, requiring filtering/smoothing that can slow responsiveness, hurting control loop stability. Or, with less filtering, the noise
can easily be "amplified" by the derivative component in the PID motor torque controller, causing audible (and tactile) humming/buzzing.

There is also apparently a somewhat common issue with the internal ADC locking up, requiring a device reset (see datasheet section 5.6). From a random sample of 4 Adafruit breakout boards, 2 of them (50%) appear to be extremely prone to this - locking up within a minute or so of use! Further testing is planned, but this may be a deal-breaker.

As of 2022-02-08, there is limited availability of this IC.

[Datasheet](https://www.mouser.com/datasheet/2/196/Infineon_TLV493D_A1B6_DataSheet_v01_10_EN-1227967.pdf)

#### MT6701 (MagnTek)
Very promising based on the datasheet and initial prototyping.

 - Haven't testing in a full control loop yet, but data looks good and relatively noise-free.
 - Lots of IO options - SPI, I2C, and ABZ - should offer good response latency.
 - No power-down or low-power options - may not be ideal for battery-powered devices
 - Not available from US distributors (Mouser, Digi-Key)

[Datasheet](http://www.magntek.com.cn/upload/MT6701_Rev.1.5.pdf)

[Ordering (LCSC)](https://lcsc.com/product-detail/Angle-Linear-Position-Sensors_Magn-Tek-MT6701CT-STD_C2856764.html)

### Motor drivers
#### TMC6300-LA
This is a relatively new IC and it's a perfect match! There generally aren't any other drivers (with integrated fets) that meet the requirements for the low-voltage and low-current motors used in this project.

Highlights:
 - 2-11V DC motor supply input
 - Up to 1.2A RMS
 - Tiny (3x3mm QFN)

 [Datasheet](https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC6300_datasheet_rev1.07.pdf)

 [Product page](https://www.trinamic.com/products/integrated-circuits/details/tmc6300-la/)

### Motors
TODO

# Firmware
TODO

# License

This project is licensed under Apache v2 (see [LICENSE.txt](LICENSE.txt)).

    Copyright 2022 Scott Bezek
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

