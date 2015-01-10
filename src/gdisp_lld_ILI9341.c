/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/ILI9341/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for
 * 			the ILI9341 and compatible HVGA display
 */

#include "gfx.h"

#if GFX_USE_GDISP

#if defined(GDISP_SCREEN_HEIGHT)
	#warning "GDISP: This low level driver does not support setting a screen size. It is being ignored."
	#undef GISP_SCREEN_HEIGHT
#endif
#if defined(GDISP_SCREEN_WIDTH)
	#warning "GDISP: This low level driver does not support setting a screen size. It is being ignored."
	#undef GDISP_SCREEN_WIDTH
#endif

#include "gdisp_lld_config.h"
#include "src/gdisp/driver.h"
#include "stm32f429i_discovery_lcd.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT		320
#endif
#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH		240
#endif
#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	50
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	100
#endif

#include "drivers/gdisp/ILI9341/ILI9341.h"
#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
static inline void init_board(GDisplay *g) {
	g->board = 0;

	/* Init LCD and LTCD. Enable layer1 only. */
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, LCD_FRAME_BUFFER);
	BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER, LCD_FRAME_BUFFER + BUFFER_OFFSET);
	BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
}

static inline void set_backlight(GDisplay *g, uint8_t percent) {
	(void) g;
	uint16_t i = (percent * 0xFF) / 100;
	BSP_LCD_SetTransparency(LCD_FOREGROUND_LAYER, (uint8_t)i);
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
	// No private area for this controller
	g->priv = 0;

	// Initialise the board interface
	init_board(g);

	/* Turn on the back-light */
	set_backlight(g, GDISP_INITIAL_BACKLIGHT);

	/* Initialise the GDISP structure */
	g->g.Width = GDISP_SCREEN_WIDTH;
	g->g.Height = GDISP_SCREEN_HEIGHT;
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
	g->g.Contrast = GDISP_INITIAL_CONTRAST;
	return TRUE;
}

#if GDISP_HARDWARE_DRAWPIXEL
LLDSPEC void gdisp_lld_draw_pixel(GDisplay *g) {
	coord_t		x, y;

	switch(g->g.Orientation) {
	default:
	case GDISP_ROTATE_0:
		x = g->p.x;
		y = g->p.y;
		break;
	case GDISP_ROTATE_90:
		x = g->p.y;
		y = GDISP_SCREEN_HEIGHT-1 - g->p.x;
		break;
	case GDISP_ROTATE_180:
		x = GDISP_SCREEN_WIDTH-1 - g->p.x;
		y = GDISP_SCREEN_HEIGHT-1 - g->p.y;
		break;
	case GDISP_ROTATE_270:
		x = GDISP_SCREEN_HEIGHT-1 - g->p.y;
		y = g->p.x;
		break;
	}
	BSP_LCD_SetTextColor(0xFF000000 | g->p.color);
	BSP_LCD_DrawHLine(x, y, 1);
}
#endif

#if GDISP_HARDWARE_FILLS
LLDSPEC void gdisp_lld_fill_area(GDisplay *g) {
	BSP_LCD_SetTextColor(0xFF000000 | g->p.color);
	BSP_LCD_FillRect(g->p.x, g->p.y, g->p.cx, g->p.cy);
}
#endif

#if GDISP_HARDWARE_CLEARS
LLDSPEC	void gdisp_lld_clear(GDisplay *g) {
	BSP_LCD_Clear(0xFF000000 | g->p.color);
}
#endif

#endif /* GFX_USE_GDISP */
