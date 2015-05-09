/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include "stm32f4xx_fmc.h"
#include "stm32f429i_discovery_sdram.h"
#include <string.h>

#define SPI_PORT	&SPID5
#define DC_PORT		GPIOD
#define DC_PIN		GPIOD_LCD_WRX

static const SPIConfig spi_cfg = {
	NULL,
	GPIOC,
	GPIOC_SPI5_LCD_CS,
	((1 << 3) & SPI_CR1_BR) | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR
};

static const ltdcConfig driverCfg = {
	240, 320,
	10, 2,
	20, 2,
	10, 4,
	0,
	0x000000,
	{
		(LLDCOLOR_TYPE *)SDRAM_BANK_ADDR,	// frame
		240, 320,							// width, height
		240 * LTDC_PIXELBYTES,				// pitch
		LTDC_PIXELFORMAT,					// fmt
		0, 0,								// x, y
		240, 320,							// cx, cy
		LTDC_COLOR_FUCHSIA,					// defcolor
		0x980088,							// keycolor
		LTDC_BLEND_FIX1_FIX2,				// blending
		0,									// palette
		0,									// palettelen
		0xFF,								// alpha
		LTDC_LEF_ENABLE						// flags
	},
	LTDC_UNUSED_LAYER_CONFIG
};

static inline void init_board(GDisplay *g) {

	// As we are not using multiple displays we set g->board to NULL as we don't use it.
	g->board = 0;

	switch(g->controllerdisplay) {
	case 0:											// Set up for Display 0
		palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7));		// UART_TX
		palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));	// UART_RX
		palSetPadMode(GPIOF, GPIOF_LCD_DCX, PAL_MODE_ALTERNATE(5));
		palSetPadMode(GPIOF, GPIOF_LCD_DE, PAL_MODE_ALTERNATE(14));

		#define STM32_SAISRC_NOCLOCK    (0 << 23)   /**< No clock.                  */
		#define STM32_SAISRC_PLL        (1 << 23)   /**< SAI_CKIN is PLL.           */
		#define STM32_SAIR_DIV2         (0 << 16)   /**< R divided by 2.            */
		#define STM32_SAIR_DIV4         (1 << 16)   /**< R divided by 4.            */
		#define STM32_SAIR_DIV8         (2 << 16)   /**< R divided by 8.            */
		#define STM32_SAIR_DIV16        (3 << 16)   /**< R divided by 16.           */

		#define STM32_PLLSAIN_VALUE                 192
		#define STM32_PLLSAIQ_VALUE                 7
		#define STM32_PLLSAIR_VALUE                 4
		#define STM32_PLLSAIR_POST                  STM32_SAIR_DIV4

		/* PLLSAI activation.*/
		RCC->PLLSAICFGR = (STM32_PLLSAIN_VALUE << 6) | (STM32_PLLSAIR_VALUE << 28) | (STM32_PLLSAIQ_VALUE << 24);
		RCC->DCKCFGR = (RCC->DCKCFGR & ~RCC_DCKCFGR_PLLSAIDIVR) | STM32_PLLSAIR_POST;
		RCC->CR |= RCC_CR_PLLSAION;

		// Initialise the SDRAM
		SDRAM_Init();

		// Clear the SDRAM
		memset((void *)SDRAM_BANK_ADDR, 0, 0x400000);

		spiStart(SPI_PORT, &spi_cfg);
		break;
	}
}

static inline void post_init_board(GDisplay *g) {
	(void) g;
}

static inline void set_backlight(GDisplay *g, uint8_t percent) {
	(void) g;
	(void) percent;
}

static inline void acquire_bus(GDisplay *g) {
	(void) g;

	spiSelect(SPI_PORT);
}

static inline void release_bus(GDisplay *g) {
	(void) g;

	spiUnselect(SPI_PORT);
}

static inline void write_index(GDisplay *g, uint8_t index) {
	static uint8_t	sindex;
	(void) g;

	palClearPad(DC_PORT, DC_PIN);
	sindex = index;
	spiSend(SPI_PORT, 1, &sindex);
}

static inline void write_data(GDisplay *g, uint8_t data) {
	static uint8_t	sdata;
	(void) g;

	palSetPad(DC_PORT, DC_PIN);
	sdata = data;
	spiSend(SPI_PORT, 1, &sdata);
}

#endif /* _GDISP_LLD_BOARD_H */
