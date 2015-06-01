uRock
=====

This is a Project about Digital Guitar Effect Pedal running on ARM Cortex-M4.
1.1 version featuring better sound quality. Require more external compoents.
For minimal cost, reference to 0.1 build.

本專案是基於ARM Cortex-M4F的開源吉他效果器
1.1版著重在聲音品質提升，需要較多的外部原件
若您想要用做簡單的方式製作，可以參考0.1版

The Source Code for 
 - Effects are under src/audio-effects/
 - Ui control under  src/ui/

程式碼架構
 - 效果：src/audio-effects/
 - 使用者界面：src/ui

Presentations & Dev-Reference / 發表會簡報與開發記錄
-------------
 - http://www.slideshare.net/zongfanyang/sitcon2015-slides-open
 - http://www.slideshare.net/zongfanyang/urokc-jserv-course-final

 - http://urock.hackpad.com

The Hardware to evaluate this code / 製作本效果器所需的硬體
-------------------------

- STM32F429I-DISCOVERY
- Self build external ADC/DAC board
- Self build ultra low noise power supply

Self Build Boards' Schematic will be released soon

- STM32F429I-DISCOVERY
- 自製 ADC/DAC 轉換板
- 自制超低底噪電源模組

自制板的線路圖將在整理後公開

The software require to build / 如何組建這份軟體
-----------------------------

* make
* arm-gcc-none-eabi 4.8 with hard floating point support, Suggest to Linaro Version
* openocd or st-flash

Tested on MAC OSX and Ubuntu 14.14.

* make
* arm-gcc-none-eabi 4.8 需要支援hard floating，建議使用Linaro版本
* openocd or st-flash

已經在Mac OSX與Ubuntu 14.14測試成功

The building Process / 組建流程
--------------------

1. make
2. plug development board
3. make openocd_flash or make flash
4. Done

1. make
2. 插上開發版
3. make openocd_flash 或 make flash
4. 完成


Free or OpenSource Sofware included as Library / 以引入作為函式庫的開源與自由軟體
----------------------------------------------
FreeRTOS
STM32 Cube Firmware
uGFX
ARM DSP Library
FATFS

License / 授權
-------

This software is licensed under MIT license, be free to modify, reuse or redistribute.

For futhuer notice please reference the LICENSE file.

本軟體使用MIT授權，可自由修改，使用或是發佈

更多資訊請詳見LICENSE檔案


Copyright (c) 2014,2015 sonicyang, anarchih, The KK, Shinshipower, 大中天
