/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"

static const ltdcConfig driverCfg = {
	240, 320,
	10, 2,
	20, 2,
	10, 4,
	0,
	0x000000,
	{
		(LLDCOLOR_TYPE *)0xD0000000,	// frame
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
	g->board = 0;

    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, LCD_FRAME_BUFFER);
    BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER, LCD_FRAME_BUFFER + BUFFER_OFFSET);
    BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
}

static inline void post_init_board(GDisplay *g) {
}

static inline void set_backlight(GDisplay *g, uint8_t percent) {
	(void) g;
	uint16_t i = (percent * 0xFF) / 100;
	BSP_LCD_SetTransparency(LCD_FOREGROUND_LAYER, (uint8_t)i);
}

static inline void acquire_bus(GDisplay *g) {
}

static inline void release_bus(GDisplay *g) {
}

static inline void write_index(GDisplay *g, uint8_t index) {
}

static inline void write_data(GDisplay *g, uint8_t data) {
}

#endif /* _GDISP_LLD_BOARD_H */
