# Ringlight

ringlight is a LED ringlight for the Wild M3B stereo microscope. It it is mounted using the objective and does not stick out from the shape of the Wild M3B. ringlight requires 2 PCBs, one for the control and one aluminum core PCB (normal PCB should be also sufficient) for the LEDs. A spacer for mounting can be 3D printed.
The inner diameter of the LED PCB is chosen so that a photography polarization filter could be placed inside, but this is not tested. There are also mounting threads for a lens, intended to be 3D printed using clear resin with the option to mount a polarization filter foil onto the LED side. This is also not tested.

Feature:
- 24x Cree J Series 2835 high-CRI LEDs
- 10-step brightness control
- full circle, half circle and quarter circle modes with adjustable section size
- more configurations using SCPI pc-based control via USB

## RP2040 toolchain

Clone ringlight next to pico folder containing pico-examples, pico-sdk and picotool.

- Clone dependencies: `git submodule init` and `git submodule update`
- Create build folder in firmware and cd into. `cd fw && mkdir build && cd build`
- Then run `export PICO_SDK_PATH=../../../pico/pico-sdk` and `cmake ..`
- Then `make -j4`
- If compilation fails with USB descriptor length errors, do the following, cd into pico-sdk folder and `cd lib/tinyusb/` and then `git pull origin master`
- Then `../load.sh` with the RP2040 connected via USB. Press the button before pluggin in. (This is not needed when running the firmware. Make sure that picotool is build and you have set up the udev rules for the pico [udev rules](https://gist.github.com/alejoseb/c7a7b4c67f0cf665dadabb26a5a87597) and the ringlight:
```
/etc/udev/rules.d/99-ringlight.rules
SUBSYSTEMS=="usb", ACTION=="add", ATTRS{idVendor}=="16c0", ATTRS{idProduct}=="03e8",  MODE="0666", GROUP="usbtmc"
```

For details see the [Getting started](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) guide.

## Resources

- Microcontroller
  - [RP2040](https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html), [Pi pico](https://www.raspberrypi.com/products/raspberry-pi-pico/), RP2040 Minimal Kicad reference design
  - [SCPI specifications](https://www.ivifoundation.org/specifications/default.aspx), [SCPI-99 standard](https://www.ivifoundation.org/docs/scpi-99.pdf), [SCPI library](https://github.com/j123b567/scpi-parser)
  - [tinyusb for usbtmc](https://github.com/hathach/tinyusb), [example](https://github.com/markb139/pico_logic)
