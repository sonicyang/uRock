uRock
=====

This is a Project about Digital Guitar Effect Pedal on ARM Cortex-M4.
The design featuring minimal design. Only require additional level shifter.

The Source Code for 
 - Effects are under src/audio-effects/
 - Ui control under  src/ui/


The Hardware to evaluate this code
-------------------------

- STM32F429I-DISCOVERY
- Self Build level shifter


The software require to build
-----------------------------

* make
* arm-gcc-none-eabi 4.8 with hard floating point support, Suggest to Linaro Version
* openocd or st-flash

The building Process
--------------------

1. make
2. plug development board
3. make openocd_flash or make flash
4. Done


Free or OpenSource Sofware included as Library
----------------------------------------------
FreeRTOS
STM32 Cube Firmware
uGFX
ARM DSP Library
FATFS

License
-------

This software is licensed under MIT license, be free to modify, reuse or redistribute.

For futhuer notice please reference the LICENSE file.



Copyright (c) 2014,2015 sonicyang, anarchih, The KK
