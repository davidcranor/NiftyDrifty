## Synopsis

This repository contains documentation and code for the electric drift trike built by Matt Nowicki, Christian Moore, Dan Meana, Kuji Nakano, and David Cranor during April/May 2017.  It's a hobby build using spare parts, random things from Taobao, copious amounts of Tsingtao, and a little bit of custom machine work to make it all fit together so there aren't really any upfront design docs.  Additional documentation will be added when (if...?) it is produced.

# Features

- Something
- Something else

## Notes on firmware

At this point, the (seriously hacky) code implements a frequency counter from the wheelmotor's hall effect feedback line for getting a sense of wheel speed as well as a lightshow for the onboard SK9822 smart LEDs.  Although less commonly found than WS2812, these LEDs are driven using a full SPI bus so they are much less timing sensitive and don't need to clobber the microcontroller's interrupt system in order to work properly.

- Written for ATMega328P using the Arduino API/bootloader
- Project set up for PlatformIO but the project files should retain compatibility with the vanilla Arduino IDE
- Project uses FastLED library for lighting effects and BasicTerm library for nicely formatted terminal debug output.
- There's a bug where the frequency counting ISR triggers spuriously, we decoupled the sensor signal with a 0.1uF cap and it looks pretty clean when scoped but the problem persists so need to take a closer look
- Due to some last-minute hardware mods (read: Kuji flipping the thing, ripping off the spoiler, and deciding that it should have been shorter anyway), some LEDs were removed and the 5.28.2017 version of the code is the only one that was completely working when cranor left shenzhen on 5.29.2017.  This version cycles through various color hues on the underlighting, and speed reading is not actually used for anything.  Previous versions contain code for using the speed readout to control color and it should be fairly simple to integrate.

## Setup

- Hall effect sensor should be connected to digital pin 2 on the arduino
- There should be an 0.1uF cap to ground on the sensor line in order to decouple noise and get a clean reading
- LED strip uses arduino pins 11 and 13 for MOSI and SCK (the hardware SPI pins on ATMega328P)
- Bluetooth speaker can be ziptied to rear spoiler to enable driver to enjoy inspirational drifting music

## TODO

- Fix interrupt triggering bug
- Add IMU and associated lighting effects
- Implement RPM counting using additional sensor lines for greater speed resolution (the motor has 3, we're currently only using one)
- Add BOM and other docs to github repo
- Moar animations and pretty colors
- General cleanup

## License

A short snippet describing the license (MIT, Apache, etc.)
