/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    boards/addons/ginput/touch/ADS7843/ginput_lld_mouse_board_olimex_stm32_e407.h
 * @brief   GINPUT Touch low level driver source for the ADS7843 on an Olimex STM32E407.
 *
 * @note	This file contains a mix of hardware specific and operating system specific
 *			code. You will need to change it for your CPU and/or operating system.
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

static const SPIConfig spicfg = { 
    0,
	GPIOG, 
    10,
    /* SPI_CR1_BR_2 |*/ SPI_CR1_BR_1 | SPI_CR1_BR_0,
};

/**
 * @brief   Initialise the board for the touch.
 *
 * @notapi
 */
static inline void init_board(void) {
	spiStart(&SPID2, &spicfg);
}

/**
 * @brief   Check whether the surface is currently touched
 * @return	TRUE if the surface is currently touched
 *
 * @notapi
 */
static inline bool_t getpin_pressed(void) {
	return (!palReadPad(GPIOG, 0));
}
/**
 * @brief   Aquire the bus ready for readings
 *
 * @notapi
 */
static inline void aquire_bus(void) {
	spiAcquireBus(&SPID2);
    //TOUCHSCREEN_SPI_PROLOGUE();
    palClearPad(GPIOG, 10);
}

/**
 * @brief   Release the bus after readings
 *
 * @notapi
 */
static inline void release_bus(void) {
	palSetPad(GPIOG, 10);
	spiReleaseBus(&SPID2);
    //TOUCHSCREEN_SPI_EPILOGUE();
}

/**
 * @brief   Read a value from touch controller
 * @return	The value read from the controller
 *
 * params[in] port	The controller port to read.
 *
 * @notapi
 */
static inline uint16_t read_value(uint16_t port) {
    static uint8_t txbuf[3] = {0};
    static uint8_t rxbuf[3] = {0};
    uint16_t ret;

    txbuf[0] = port;

    spiExchange(&SPID2, 3, txbuf, rxbuf);

    ret = (rxbuf[1] << 5) | (rxbuf[2] >> 3); 
    
	return ret;
}

#endif /* _GINPUT_LLD_MOUSE_BOARD_H */

