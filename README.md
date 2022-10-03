# SmartKnob
SmartKnob is an open-source input device with software-configurable endstops and virtual detents.

A brushless gimbal motor is paired with a magnetic encoder to provide closed-loop torque feedback control, making it
possible to dynamically create and adjust the feel of detents and endstops.

[![Build Status](https://github.com/scottbez1/smartknob/actions/workflows/electronics.yml/badge.svg?branch=master)](https://github.com/scottbez1/smartknob/actions/workflows/electronics.yml)
[![Build Status](https://github.com/scottbez1/smartknob/actions/workflows/pio.yml/badge.svg?branch=master)](https://github.com/scottbez1/smartknob/actions/workflows/pio.yml)

# Designs

## SmartKnob View
Premium SmartKnob experience. Under active development.

🎉 **Motors are [now available](https://www.sparkfun.com/products/20441)!** If you've been following this project,
you'll know that the recommended motors went out of stock nearly immediately after it was published.
Thanks to [the community](https://github.com/scottbez1/smartknob/issues/16#issuecomment-1094482805%5D), we were able to
identify the likely original manufacturer, and recently SparkFun Electronics funded a new production run and are now
[selling them](https://www.sparkfun.com/products/20441)! Thanks to everyone who helped search and investigate different
motor options along the way!

Features:
 - 240x240 round LCD ("GC9A01"), protected by 39.5mm watch glass on rotor
 - BLDC gimbal motor, with a hollow shaft for mechanically & electrically connecting the LCD
 - Powered by ESP32-PICO-V3-02 (Lilygo TMicro32 Plus module)
 - PCB flexure and strain gauges used for press detection (haptic feedback provided via the motor)
 - 8 side-firing RGB LEDs (SK6812-SIDE-A) illuminate ring around the knob
 - USB-C (2.0) connector for 5V power and serial data/programming (CH340)
 - VEML7700 ambient light sensor for automatic backlight & LED intensity adjustment
 - Versatile back plate for mounting - use either 4x screws, or 2x 3M medium Command strips (with cutouts for accessing removal tabs after installation)
 - Front cover snaps on for easy access to the PCB

**Current status:** Not recommended for general use (mechanical and electrical revisions may be needed depending on motor/electronics availability)

### Demo video

<a href="https://www.youtube.com/watch?v=ip641WmY4pA">
    <img src="https://img.youtube.com/vi/ip641WmY4pA/maxresdefault.jpg" width="480" />
</a>

### How it works
<a href="https://www.youtube.com/watch?v=Q76dMggUH1M">
    <img src="https://img.youtube.com/vi/Q76dMggUH1M/maxresdefault.jpg" width="480" />
</a>

### 3D CAD

![Exploded view](doc/img/explodedv145.gif)

Latest Fusion 360 Model: https://a360.co/3BzkU0n

### Build your own?

While this is a "DIY" open-source project, it is not yet a mature plug-and-play project. If you intend to build your own, note that it requires advanced soldering experience to build - very small-pitch surface-mount soldering is required (reflow or hot air recommended), and assembly is quite time-consuming and delicate. Please go into it with the expectation that you will almost certainly need to be able to troubleshoot some hardware and firmware issues yourself - I recommend reviewing/understanding the schematics and basic firmware before jumping in!

More documentation on the BOM and what parts you need to order is coming in the future - thanks so much for your interest! Follow me on [Twitter](https://twitter.com/scottbez1) for the latest updates on this and other projects.

View the latest auto-generated (untested) [Base PCB Interactive BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-ibom.html) and [Screen PCB Interactive BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-ibom.html) (or, the combined [BOM csv](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-bom.csv)) for electronics/hardware parts list. ⚠️ These are auto-generated from the latest untested revision on GitHub. For tested/stable/recommended artifacts, use a [release](https://github.com/scottbez1/smartknob/releases) instead.

A few miscellaneous notes in the meantime:

 - This can _probably_ be FDM 3D printed with a well-tuned printer, but the parts shown in videos/photos were MJF printed in nylon for tight tolerances and better surface finish
 - If you wanted a simpler build, you could omit the LCD and just merge the knob + glass from the model into a single STL to get a closed-top knob
 - There's limited space inside the LCD mount for wiring, and 8 wires need to fit through the hole in the center. I used 30 AWG wire-wrapping wire. Enamel-coated wire would probably work too.
 - Strain gauges are BF350-3AA, and glued in place with CA glue (I'll include video of this process in the future, but essentially I used kapton tape to pick up the strain gauge and hold it in place during curing). This has to be done after reflow soldering, and would be hard to remove/fix in case of a mistake, so MAKE SURE TO PRACTICE GLUING strain gauges to other items before attempting on the PCB!
 - The TMC6300 is _tiny_ and has a bottom pad, so I would seriously consider getting a stencil along with the PCB order. Even with the stencil I needed to manually clean up some bridging afterward; I _highly_ recommend Chip Quik NC191 gel flux, available on [Amazon](https://amzn.to/3MGDSr5) (or use this [non-affiliate link](https://www.amazon.com/Smooth-Flow-No-Clean-syringe-plunger/dp/B08KJPG3NZ) instead) or from your electronics distributor of choice. Flux is also very helpful when soldering the LCD ribbon cable to to screen PCB.
 - For breadboard prototyping, the [TMC6300-BOB](https://www.trinamic.com/support/eval-kits/details/tmc6300-bob/) is awesome and way easier to work with than the bare chip if you just want to play around with low current BLDC motors
 - For AliExpress purchases: I highly recommend **only** using AliExpress Standard Shipping (purchasing in the US). I have had multiple purchases take months or never get delivered when purchased with Cainiao or other low cost shipping options, whereas AliExpress Standard is very reliable and generally faster in my experience.
 - Make sure to check the [open issues](https://github.com/scottbez1/smartknob/issues) - this design is not yet "stable", so beware that everything may not go smoothly. I would not recommend ordering these parts yourself until the [stable release v1.0 milestone](https://github.com/scottbez1/smartknob/milestone/1) is complete, as there are some mechanical interference issues in the current revision.

Future plans:
 - consider switch to using an ESP32-S3-MINI-1 module (once Arduino core support is complete), as that would allow for direct USB HID support (for joystick/macro-pad type input to a computer)
 - Bluetooth HID support?
 - get wifi configured and working (probably MQTT?). Currently memory is an issue with the full display framebuffer sprite. PSRAM might fix this (requires newer ESP-IDF & unreleased Arduino core, and from a brief test I got horrible performance with PSRAM enabled), or the next item might help reduce memory:
 - migrate to LVGL, for better display rendering and easy support for menus, etc. Shouldn't require a full 240x240x24b framebuffer in memory, freeing some for wifi, etc.
 - integrate nanopb for structured serial data (see [splitflap protobuf protocol](https://github.com/scottbez1/splitflap/blob/1440aba54d5b0d822ec5da68762431879988d7ef/arduino/splitflap/esp32/splitflap/serial_proto_protocol.cpp) for example)
 - Home Assistant integration, or other real-world applications
 - ???
 - [Profit](https://github.com/sponsors/scottbez1/) 😉


#### Base PCB

<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-front-3d.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-front-3d.png" width="300" />
</a>
<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-back-3d.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-back-3d.png" width="300" />
</a>

Ordering notes: use white soldermask, for reflecting light from RGB LED ring around the knob. Should be 1.2mm thick (not "standard" 1.6mm).

If you are ordering a stencil for solder paste from JLCPCB and plan to apply paste by hand (as
[shown in the video](https://youtu.be/Q76dMggUH1M?t=372)) without a stencil frame/machine, make sure to select
**"Customized size"** and enter smaller dimensions (e.g. 100mm x 100mm) to avoid getting a much larger stencil than you
need. This will also likely reduce the cost of shipping substantially! Also, select only the **Top** side; the bottom
only has 2 SMT components - the motor connector and VEML7700 ALS - so it's not worth getting a stencil for that.

Latest auto-generated (untested and likely broken!) artifacts⚠️:

[Schematic](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-schematic.pdf)

[Interactive BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-ibom.html)

[PCB Packet](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-pcb-packet.pdf)

[Gerbers](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-jlc/gerbers.zip)

⚠️ For tested/stable/recommended artifacts, use a [release](https://github.com/scottbez1/smartknob/releases) instead.

#### Screen PCB

<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-front-3d.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-front-3d.png" width="300" />
</a>
<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-back-3d.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-back-3d.png" width="300" />
</a>

Ordering notes: Must be 1.2mm thick (not "standard" 1.6mm) per mechanical design.

There are few enough components on the Screen PCB that I chose to hand-solder them rather than reflow with solder paste
and a stencil, but if you order a stencil, see the note above about selecting a "Customized size" to be easier to
handle and save on shipping. Also make sure to select the **Bottom** side only; all the components are on the bottom
side of the screen PCB.

Latest auto-generated (untested and likely broken!) artifacts⚠️:

[Schematic](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-schematic.pdf)

[Interactive BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-ibom.html)

[PCB Packet](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-pcb-packet.pdf)

[Gerbers](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-jlc/gerbers.zip)

⚠️ For tested/stable/recommended artifacts, use a [release](https://github.com/scottbez1/smartknob/releases) instead.


## SmartKnob Mini
Planned for the future.


# Frequently Asked Questions (FAQ)

**How much does it cost?**

I wish I could tell you now, but I don't actually know off the top of my head. Check back soon - I've only built 1 so far, which was the result of a bunch of tinkering and prototyping over an extended period, so I don't have all the expenses tallied up yet. Certainly less than $200 in parts, and maybe closer to $100?

**Does it work with XYZ?**

Not yet. So far I've only implemented enough firmware for the demo shown in the video, so you can't actually use it for anything productive yet. The basic detent configuration API is there, but not much else. Lots of firmware work remains to be done. If you build one, I'd love your help adding support for XYZ though!

**Can I buy one as a kit or already assembled?**

Probably not? Or at least, I don't have any immediate plans to sell them myself. It's not that I don't want you to be happy, but hardware is a hard business and I just work on this stuff in my free time.

It's open source with a fairly permissive license though, so in theory anyone could start offering kits/assemblies. If someone does go down that route of selling them, note that attribution is
 _required_ (and I wouldn't say no to [royalties/tips/thanks](https://github.com/sponsors/scottbez1/) if you're in a giving mood 🙂).


## General Component Info

### Magnetic encoders

#### MT6701 (MagnTek)
Excellent sensor at a reasonable price - highly recommended. Less noisy than TLV493D, and more responsive (control loop is more stable) using SSI.

 - Lots of IO options - SSI, I2C, and ABZ - should offer good response latency
 - SSI includes CRC to validate data
 - No power-down or low-power options - may not be ideal for battery-powered devices
 - Not available from US distributors (Mouser, Digi-Key)

[Datasheet](http://www.magntek.com.cn/upload/MT6701_Rev.1.5.pdf)

[Ordering (LCSC)](https://lcsc.com/product-detail/Angle-Linear-Position-Sensors_Magn-Tek-MT6701CT-STD_C2856764.html)

#### TLV493D (Infineon)
A mediocre choice. Easy to prototype with using [Adafruit's QWIIC breakout board](https://www.adafruit.com/product/4366).

In my testing, it is a little noisy, requiring filtering/smoothing that can slow responsiveness, hurting control loop stability. Or, with less filtering, the noise
can easily be "amplified" by the derivative component in the PID motor torque controller, causing audible (and tactile) humming/buzzing.

There is also apparently a known silicon issue that causes the internal ADC to sometimes completely lock up, requiring a full reset and re-configuration. See section
5.6 in the [User Manual](https://www.infineon.com/dgdl/Infineon-TLV493D-A1B6_3DMagnetic-UM-v01_03-EN.pdf?fileId=5546d46261d5e6820161e75721903ddd)

    In the Master Controlled Mode (MCM) or the Fast Mode (FM) the ADC conversion may hang up. A hang up can
    be detected by:
     - Frame Counter (FRM) counter stucks and does not increment anymore.

In my experience testing 4 different Adafruit breakout boards, 2 of them (50%) regularly exhibit this lockup behavior within a minute or two of use. It is possible to detect and auto-reset (and there is code in the project to do so), but it is slow and may cause undesirable jumps/delays if the sensor locks up often.

[Datasheet](https://www.mouser.com/datasheet/2/196/Infineon_TLV493D_A1B6_DataSheet_v01_10_EN-1227967.pdf)


#### AS5600 (AMS)
A mediocre choice. Cheap breakout boards are readily available.

In my testing, it's fairly noisy (anecdotally, noisier than the TLV493d), requiring filtering/smoothing that can slow responsiveness, hurting control loop stability. Additionally, it saturates at a lower magnetic field strength than other sensors I tested, requiring a significant air gap (8-10mm) when used with a strong neodymium diametric magnet like [Radial Magnets 8995](https://www.digikey.com/en/products/detail/radial-magnets-inc/8995/5126077).

[Datasheet](https://ams.com/documents/20143/36005/AS5600_DS000365_5-00.pdf)

### Motor drivers
#### TMC6300-LA
This is a relatively new IC and it's a perfect match! There generally aren't any other drivers (with integrated fets) that meet the requirements for the low-voltage and low-current motors used in this project (DRV8316 might work, but has not been tested).

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

Available [from SparkFun](https://www.sparkfun.com/products/20441)!

# Firmware
TODO: document this

Also TODO: implement a lot more of the firmware

# Acknowledgements
This project was greatly inspired by Jesse Schoch's video "[haptic textures and virtual detents](https://www.youtube.com/watch?v=1gPQfDkX3BU)" and the
corresponding [discussion in the SimpleFOC community](https://community.simplefoc.com/t/haptic-textures/301). Seriously, this project wouldn't exist if not for that video - thank you Jesse!


# License

This project is licensed under Apache v2 (software, electronics, documentation) and Creative Commons Attribution 4.0 (hardware/mechanical) (see [LICENSE.txt](LICENSE.txt) and [Creative Commons](https://creativecommons.org/licenses/by/4.0/)).

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

