/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    boards/addons/ginput/touch/ADS7843/ginput_lld_mouse_board_st_stm32f4_discovery.h
 * @brief   GINPUT Touch low level driver source for the ADS7843 on an st_stm32f4_discovery.
 *
 * @note	This file contains a mix of hardware specific and operating system specific
 *			code. You will need to change it for your CPU and/or operating system.
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

/*
 * uGFX port used:  ChibiOS/RT
 *
 * SCK:             GPIOB, 13
 * MISO:            GPIOB, 14
 * MOSI:            GPIOB, 15
 * CS:              GPIOC, 4
 * IRC:             GPIOC, 5
 *
 * Note that you can tweak the SPI bus speed
 */
static const SPIConfig spicfg = { 
    0,
	GPIOC,
    4,
    /* SPI_CR1_BR_2 |*/ SPI_CR1_BR_1 | SPI_CR1_BR_0,
};

static inline void init_board(void) {
	palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5) );	/* SCK */
	palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5) );	/* MISO */
	palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5) );	/* MOSI */
	palSetPadMode(GPIOC,  4, PAL_MODE_OUTPUT_PUSHPULL);	/* CS */

	spiStart(&SPID2, &spicfg);
}

static inline bool_t getpin_pressed(void) {
	return (!palReadPad(GPIOC, 5));
}

static inline void aquire_bus(void) {
	spiAcquireBus(&SPID2);
	palClearPad(GPIOC, 4);
}

static inline void release_bus(void) {
	palSetPad(GPIOC, 4);
	spiReleaseBus(&SPID2);
}

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

