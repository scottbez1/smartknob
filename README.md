# SmartKnob

Coming soon...

# Hardware

## Designs

### SmartKnob View
Premium SmartKnob experience. Under active development. Untested & unproven.

![Exploded view](doc/img/explodedv145.gif)

<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-3D_top.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-3D_top.png" width="300" />
</a>
<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-3D_bottom.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-3D_bottom.png" width="300" />
</a>


[Schematic](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-schematic.pdf)

[Interactive BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-ibom.html)


### SmartKnob Handheld
An exploration of a small handheld form-factor. Under active development.

<a href="doc/img/PXL_20220212_013249074.jpg"><img src="doc/img/PXL_20220212_013249074.jpg" width="200" /></a>

### SmartKnob Mini
Planned for the future.

## Component Info

### Magnetic encoders
#### TLV493D (Infineon)
A mediocre choice. Easy to prototype with using [Adafruit's QWIIC breakout board](https://www.adafruit.com/product/4366).

In my testing, it is a little noisy, requiring filtering/smoothing that can slow responsiveness, hurting control loop stability. Or, with less filtering, the noise
can easily be "amplified" by the derivative component in the PID motor torque controller, causing audible (and tactile) humming/buzzing.

There is also apparently a known silicon issue that causes the internal ADC to sometimes completely lock up, requiring a full reset and re-configuration. See section
5.6 in the [User Manual](https://www.infineon.com/dgdl/Infineon-TLV493D-A1B6_3DMagnetic-UM-v01_03-EN.pdf?fileId=5546d46261d5e6820161e75721903ddd)

    In the Master Controlled Mode (MCM) or the Fast Mode (FM) the ADC conversion may hang up. A hang up can
    be detected by:
     - Frame Counter (FRM) counter stucks and does not increment anymore.

In my experience testing 4 different Adafruit breakout boards, 2 of them (50%) regularly exhibit this lockup behavior within a minute or two of use.

As of 2022-02-08, there is limited availability of this IC.

[Datasheet](https://www.mouser.com/datasheet/2/196/Infineon_TLV493D_A1B6_DataSheet_v01_10_EN-1227967.pdf)

#### MT6701 (MagnTek)
Very promising based on the datasheet and initial tests. Seems to be less noisy than TLV493D, and more responsive (control loop is more stable) using SSI.

 - Lots of IO options - SSI, I2C, and ABZ - should offer good response latency
 - SSI include CRC to validate data
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
#### 32mm Rotor, Hollow Shaft, Diametric magnet
<a href="doc/img/motors/PXL_20220121_221746595.jpg"><img src="doc/img/motors/PXL_20220121_221746595.jpg" width="200" /></a>
<a href="doc/img/motors/PXL_20220121_221738745.jpg"><img src="doc/img/motors/PXL_20220121_221738745.jpg" width="200" /></a>


- 32mm rotor
- 15mm overall height (including magnet), 12.75mm height w/o magnet, 9mm rotor height
- low/zero cogging - excellent for completely smooth input
- 5.9mm hollow shaft
- built-in diametric magnet for encoder
- Proven option

This is overall the easiest motor to get started with. Low cogging and a built-in diametric magnet are great!

[AliExpress](https://www.aliexpress.com/item/1005001384349991.html)

#### ~17mm Rotor
<a href="doc/img/motors/PXL_20220121_220530808.jpg"><img src="doc/img/motors/PXL_20220121_220530808.jpg" width="200" /></a>
<a href="doc/img/motors/PXL_20220121_220546761.jpg"><img src="doc/img/motors/PXL_20220121_220546761.jpg" width="200" /></a>

- Not tested yet
- Stepped 17.3mm and 16.3mm diameter rotor
- 11.2mm total height, 8.6mm rotor height
- moderate cogging - not ideal for completely smooth input
- Rotor has threaded hole on the side that should be usable for keying a cap
- Very little shaft accessible on bottom - may be tricky to mount a diametric magnet to it. Maybe could replace screw with something else?

[AliExpress](https://www.aliexpress.com/item/1005003240145615.html)

# Firmware
TODO

# Acknowledgements
This project was greatly inspired by Jesse Schoch's video "[haptic textures and virtual detents](https://www.youtube.com/watch?v=1gPQfDkX3BU)" and the
corresponding [discussion in the SimpleFOC community](https://community.simplefoc.com/t/haptic-textures/301).


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

