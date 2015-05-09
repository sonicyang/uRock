/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    boards/addons/gdisp/board_SSD1306_spi.h
 * @brief   GDISP Graphic Driver subsystem board interface for the SSD1306 display.
 *
 * @note	This file contains a mix of hardware specific and operating system specific
 *			code. You will need to change it for your CPU and/or operating system.
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

// Pin & SPI setup

#define SPI_DRIVER (&SPID2)
#define SPI_PORT GPIOB
#define SCK_PAD  13
#define MISO_PAD 14
#define MOSI_PAD 15

#define CS_PORT     GPIOC
#define RESET_PORT  GPIOC
#define DNC_PORT    GPIOC
#define CS_PAD     7        // 0 = chip selected
#define RESET_PAD  8        // 0 = reset
#define DNC_PAD    9        // control=0, data=1

static SPIConfig spi_cfg = { NULL, CS_PORT, CS_PAD, 0 };

static inline void init_board(GDisplay *g) {
	(void) g;
	g->board = 0; 
	// Maximum speed of SSD1306 is 10Mhz, so set SPI speed less or = to that.  
	//
	// STM32 specific setup
	// STM32_PCLK1 is APB1 frequence in hertz.
	// STM32_PCLK2 is APB2 frequence in hertz.
	// See manual clock diagram to determine APB1 or APB2 for spi in use. 
	// SPI2 uses APB1 clock on stm32151
	// BR bits divide PCLK as follows
	// 000  /2   =   16 MHz
	// 001  /4   =   8 MHz
	// 010  /8   =   4 MHz
	// 011  /16  =   2 MHz
	// 100  /32  =   1 MHz
	// 101  /64  =   500 kHz
	// 110  /128 =   250 kHz
	// 111  /256 =   125 kHz
	unsigned long spi_clk = STM32_PCLK1 / 2;
	unsigned code = 0;
	while (spi_clk > 10000000) {
		code++;
		spi_clk /= 2;
	}
	spi_cfg.cr1 |= (code << 3);
	
	if (g->controllerdisplay == 0) {
		palSetPadMode(SPI_PORT, SCK_PAD, PAL_MODE_ALTERNATE(5)|PAL_STM32_OTYPE_PUSHPULL|PAL_STM32_OSPEED_MID2);
		palSetPadMode(SPI_PORT, MOSI_PAD, PAL_MODE_ALTERNATE(5)|PAL_STM32_OTYPE_PUSHPULL|PAL_STM32_OSPEED_MID2);
		palSetPadMode(SPI_PORT, MISO_PAD, PAL_MODE_ALTERNATE(5));
		palSetPadMode(RESET_PORT, RESET_PAD, PAL_MODE_OUTPUT_PUSHPULL);
		palSetPadMode(CS_PORT, CS_PAD, PAL_MODE_OUTPUT_PUSHPULL);
		palSetPadMode(DNC_PORT, DNC_PAD, PAL_MODE_OUTPUT_PUSHPULL);
		palSetPad(CS_PORT, CS_PAD);
		palSetPad(RESET_PORT, RESET_PAD);
		palClearPad(DNC_PORT, DNC_PAD); 
	}
}

static inline void post_init_board(GDisplay *g) {
	(void) g;
}


static inline void setpin_reset(GDisplay *g, bool_t state) {
	(void) g;
	palWritePad(RESET_PORT, RESET_PAD, !state);
}

static inline void acquire_bus(GDisplay *g) {
	(void) g;
	spiAcquireBus(SPI_DRIVER);
	spiStart(SPI_DRIVER, &spi_cfg); 
	spiSelect(SPI_DRIVER);
}

static inline void release_bus(GDisplay *g) {
	(void) g;
	spiUnselect(SPI_DRIVER);
	spiStop(SPI_DRIVER);
	spiReleaseBus(SPI_DRIVER);
}

static inline void write_cmd(GDisplay *g, uint8_t cmd) {
	(void) g;
	static uint8_t buf;
	palClearPad(DNC_PORT, DNC_PAD);
	buf = cmd;
	spiSend(SPI_DRIVER, 1, &buf);
}

static inline void write_data(GDisplay *g, uint8_t* data, uint16_t length) {
	(void) g;
	palSetPad(DNC_PORT, DNC_PAD);
	spiSend(SPI_DRIVER, length, data);
}


#endif /* _GDISP_LLD_BOARD_H */
