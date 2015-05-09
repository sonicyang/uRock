/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
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

#define GDISP_DRIVER_VMT			GDISPVMT_STM32F429iDiscovery
#include "drivers/gdisp/STM32F429iDiscovery/gdisp_lld_config.h"
#include "src/gdisp/gdisp_driver.h"

#include "stm32f429xx.h"

#define STM32F429_439xx
#include "stm32_ltdc.h"
#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"

typedef struct ltdcLayerConfig {
	// frame
	LLDCOLOR_TYPE	*frame;			// Frame buffer address
	coord_t			width, height;	// Frame size in pixels
	coord_t			pitch;			// Line pitch, in bytes
	uint16_t		fmt;			// Pixel format in LTDC format

	// window
	coord_t			x, y;			// Start pixel position of the virtual layer
	coord_t			cx, cy;			// Size of the virtual layer

	uint32_t		defcolor;		// Default color, ARGB8888
	uint32_t		keycolor;		// Color key, RGB888
	uint32_t		blending;		// Blending factors
	const uint32_t *palette;		// The palette, RGB888 (can be NULL)
	uint16_t		palettelen;		// Palette length
	uint8_t			alpha;			// Constant alpha factor
	uint8_t			layerflags;		// Layer configuration
} ltdcLayerConfig;

#define LTDC_UNUSED_LAYER_CONFIG	{ 0, 1, 1, 1, LTDC_FMT_L8, 0, 0, 1, 1, 0x000000, 0x000000, LTDC_BLEND_FIX1_FIX2, 0, 0, 0, 0 }

typedef struct ltdcConfig {
	coord_t		width, height;				// Screen size
	coord_t		hsync, vsync;				// Horizontal and Vertical sync pixels
	coord_t		hbackporch, vbackporch;		// Horizontal and Vertical back porch pixels
	coord_t		hfrontporch, vfrontporch;	// Horizontal and Vertical front porch pixels
	uint8_t		syncflags;					// Sync flags
	uint32_t	bgcolor;					// Clear screen color RGB888

	ltdcLayerConfig	bglayer;				// Background layer config
	ltdcLayerConfig	fglayer;				// Foreground layer config
} ltdcConfig;

#if GDISP_LLD_PIXELFORMAT == GDISP_PIXELFORMAT_RGB565
	#define LTDC_PIXELFORMAT	LTDC_FMT_RGB565
	#define LTDC_PIXELBYTES		2
	#define LTDC_PIXELBITS		16
#elif GDISP_LLD_PIXELFORMAT == GDISP_PIXELFORMAT_RGB888
	#define LTDC_PIXELFORMAT	LTDC_FMT_RGB888
	#define LTDC_PIXELBYTES		3
	#define LTDC_PIXELBITS		24
#else
	#error "GDISP: STM32F4iDiscovery - unsupported pixel format"
#endif

#include "board_STM32F429iDiscovery.h"

#include "ili9341.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	50
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	100
#endif

/*===========================================================================*/
/* Driver local routines    .                                                */
/*===========================================================================*/

#define PIXIL_POS(g, x, y)		((y) * driverCfg.bglayer.pitch + (x) * LTDC_PIXELBYTES)
#define PIXEL_ADDR(g, pos)		((LLDCOLOR_TYPE *)((uint8_t *)driverCfg.bglayer.frame+pos))

#define GDISP_SCREEN_HEIGHT 320
#define GDISP_SCREEN_WIDTH 320

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {

	// Initialize the private structure
	g->priv = 0;

	// Init the board
	init_board(g);

	/* Turn on the back-light */
	set_backlight(g, GDISP_INITIAL_BACKLIGHT);

	/* Initialise the GDISP structure */
	g->g.Width = driverCfg.bglayer.width;
	g->g.Height = driverCfg.bglayer.height;
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
	g->g.Contrast = GDISP_INITIAL_CONTRAST;
	return TRUE;
}

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

LLDSPEC	color_t gdisp_lld_get_pixel_color(GDisplay *g) {
	unsigned		pos;
	LLDCOLOR_TYPE	color;

	#if GDISP_NEED_CONTROL
		switch(g->g.Orientation) {
		case GDISP_ROTATE_0:
		default:
			pos = PIXIL_POS(g, g->p.x, g->p.y);
			break;
		case GDISP_ROTATE_90:
			pos = PIXIL_POS(g, g->p.y, g->g.Width-g->p.x-1);
			break;
		case GDISP_ROTATE_180:
			pos = PIXIL_POS(g, g->g.Width-g->p.x-1, g->g.Height-g->p.y-1);
			break;
		case GDISP_ROTATE_270:
			pos = PIXIL_POS(g, g->g.Height-g->p.y-1, g->p.x);
			break;
		}
	#else
		pos = PIXIL_POS(g, g->p.x, g->p.y);
	#endif

	color = PIXEL_ADDR(g, pos)[0];
	return gdispNative2Color(color);
}

#if GDISP_NEED_CONTROL
	LLDSPEC void gdisp_lld_control(GDisplay *g) {
		switch(g->p.x) {
		case GDISP_CONTROL_POWER:
			if (g->g.Powermode == (powermode_t)g->p.ptr)
				return;
			switch((powermode_t)g->p.ptr) {
			case powerOff: case powerOn: case powerSleep: case powerDeepSleep:
				// TODO
				break;
			default:
				return;
			}
			g->g.Powermode = (powermode_t)g->p.ptr;
			return;

		case GDISP_CONTROL_ORIENTATION:
			if (g->g.Orientation == (orientation_t)g->p.ptr)
				return;
			switch((orientation_t)g->p.ptr) {
				case GDISP_ROTATE_0:
				case GDISP_ROTATE_180:
					if (g->g.Orientation == GDISP_ROTATE_90 || g->g.Orientation == GDISP_ROTATE_270) {
						coord_t		tmp;

						tmp = g->g.Width;
						g->g.Width = g->g.Height;
						g->g.Height = tmp;
					}
					break;
				case GDISP_ROTATE_90:
				case GDISP_ROTATE_270:
					if (g->g.Orientation == GDISP_ROTATE_0 || g->g.Orientation == GDISP_ROTATE_180) {
						coord_t		tmp;

						tmp = g->g.Width;
						g->g.Width = g->g.Height;
						g->g.Height = tmp;
					}
					break;
				default:
					return;
			}
			g->g.Orientation = (orientation_t)g->p.ptr;
			return;

		case GDISP_CONTROL_BACKLIGHT:
			if ((unsigned)g->p.ptr > 100) g->p.ptr = (void *)100;
			set_backlight(g, (unsigned)g->p.ptr);
			g->g.Backlight = (unsigned)g->p.ptr;
			return;

		case GDISP_CONTROL_CONTRAST:
			if ((unsigned)g->p.ptr > 100) g->p.ptr = (void *)100;
			// TODO
			g->g.Contrast = (unsigned)g->p.ptr;
			return;
		}
	}

#endif

#endif /* GFX_USE_GDISP */
