/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_DRIVER_VMT			GDISPVMT_SSD1963
#include "drivers/gdisp/SSD1963/gdisp_lld_config.h"
#include "src/gdisp/driver.h"

#define CALC_PERIOD(w,b,f,p)				(p+b+w+f)
#define CALC_FPR(w,h,hb,hf,hp,vb,vf,vp,fps)	((fps * CALC_PERIOD(w,hb,hf,hp) * CALC_PERIOD(h,vb,vf,vp) * 1048576)/100000000)

typedef struct LCD_Parameters {
	coord_t		width, height;					// Panel width and height
	uint16_t	hbporch;						// Horizontal Back Porch
	uint16_t	hfporch;						// Horizontal Front Porch
	uint16_t	hpulse;							// Horizontal Pulse
	uint16_t	hperiod;						// Horizontal Period (Total)
	uint16_t	vbporch;						// Vertical Back Porch
	uint16_t	vfporch;						// Vertical Front Porch
	uint16_t	vpulse;							// Vertical Pulse
	uint16_t	vperiod;						// Vertical Period (Total)
	uint32_t	fpr;							// Calculated FPR
} LCD_Parameters;

#include "board_SSD1963.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	50
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	90
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#include "drivers/gdisp/SSD1963/ssd1963.h"

#define write_reg(g, reg, data)		{ write_index(g, reg); write_data(g, data); }
#define write_data16(g, data)		{ write_data(g, (data)>>8); write_data(g, (data) & 0xFF); }

static inline void set_viewport(GDisplay* g) {
	write_index(g, SSD1963_SET_PAGE_ADDRESS);
	write_data16(g, g->p.y);
	write_data16(g, g->p.y+g->p.cy-1);
	write_index(g, SSD1963_SET_COLUMN_ADDRESS);
	write_data16(g, g->p.x);
	write_data16(g, g->p.x+g->p.cx-1);
	write_index(g, SSD1963_WRITE_MEMORY_START);
}

/**
 * The backlight is controlled by the controller.
 */
static inline void set_backlight(GDisplay *g, uint8_t percent) {
	//duty_cycle is 00..FF
	//Work in progress: the SSD1963 has a built-in PWM, its output can
	//be used by a Dynamic Background Control or by a host (user)
	//Check your LCD's hardware, the PWM connection is default left open and instead
	//connected to a LED connection on the breakout board
	write_index(g, SSD1963_SET_PWM_CONF);		//set PWM for BackLight
	write_data(g, 0x01);
	write_data(g, (55+percent*2) & 0x00FF);
	write_data(g, 0x01);						//controlled by host (not DBC), enabled
	write_data(g, 0xFF);
	write_data(g, 0x60);						//don't let it go too dark, avoid a useless LCD
	write_data(g, 0x0F);						//prescaler ???
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
	LCD_Parameters *	lcdp;

	// The private area for this controller is the LCD timings
	lcdp = (void *)&DisplayTimings[g->controllerdisplay];
	g->priv = lcdp;

	// Initialise the board interface
	init_board(g);

	// Hardware reset
	setpin_reset(g, TRUE);
	gfxSleepMilliseconds(20);
	setpin_reset(g, FALSE);
	gfxSleepMilliseconds(20);

	// Get the bus for the following initialisation commands
	acquire_bus(g);
	
	write_index(g, SSD1963_SOFT_RESET);
	gfxSleepMilliseconds(5);

	/* Driver PLL config */
	write_index(g, SSD1963_SET_PLL_MN);
	write_data(g, 35);								// PLLclk = REFclk (10Mhz) * 36 (360Mhz)
	write_data(g, 2);								// SYSclk = PLLclk / 3  (120MHz)
	write_data(g, 4);								// Apply calculation bit, else it is ignored
	write_reg(g, SSD1963_SET_PLL, 0x01);			// Enable PLL
	gfxSleepMilliseconds(5);
	write_reg(g, SSD1963_SET_PLL, 0x03);			// Use PLL
	gfxSleepMilliseconds(5);
	write_index(g, SSD1963_SOFT_RESET);
	gfxSleepMilliseconds(5);

	/* Screen size */
	write_index(g, SSD1963_SET_GDISP_MODE);
	write_data(g, 0x18);							//Enabled dithering
	write_data(g, 0x00);
	write_data16(g, lcdp->width-1);
	write_data16(g, lcdp->height-1);
	write_data(g, 0x00);							// RGB

	write_reg(g, SSD1963_SET_PIXEL_DATA_INTERFACE, SSD1963_PDI_16BIT565);

	/* LCD Clock specs */
	write_index(g, SSD1963_SET_LSHIFT_FREQ);
	write_data(g, (lcdp->fpr >> 16) & 0xFF);
	write_data(g, (lcdp->fpr >> 8) & 0xFF);
	write_data(g, lcdp->fpr & 0xFF);

	write_index(g, SSD1963_SET_HORI_PERIOD);
	write_data16(g, lcdp->hperiod);
	write_data16(g, lcdp->hpulse + lcdp->hbporch);
	write_data(g, lcdp->hpulse);
	write_data(g, 0x00);
	write_data(g, 0x00);
	write_data(g, 0x00);

	write_index(g, SSD1963_SET_VERT_PERIOD);
	write_data16(g, lcdp->vperiod);
	write_data16(g, lcdp->vpulse + lcdp->vbporch);
	write_data(g, lcdp->vpulse);
	write_data(g, 0x00);
	write_data(g, 0x00);

	/* Tear effect indicator ON. This is used to tell the host MCU when the driver is not refreshing the panel (during front/back porch) */
	write_reg(g, SSD1963_SET_TEAR_ON, 0x00);

	/* Turn on */
	write_index(g, SSD1963_SET_DISPLAY_ON);
	
	/* Turn on the back-light */
	set_backlight(g, GDISP_INITIAL_BACKLIGHT);

	// Finish Init
	post_init_board(g);

	// Release the bus
	release_bus(g);

	/* Initialise the GDISP structure */
	g->g.Width = lcdp->width;
	g->g.Height = lcdp->height;
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
	g->g.Contrast = GDISP_INITIAL_CONTRAST;
	return TRUE;
}

#if GDISP_HARDWARE_STREAM_WRITE
	LLDSPEC	void gdisp_lld_write_start(GDisplay *g) {
		acquire_bus(g);
		set_viewport(g);
	}
	LLDSPEC	void gdisp_lld_write_color(GDisplay *g) {
		write_data(g, gdispColor2Native(g->p.color));
	}
	LLDSPEC	void gdisp_lld_write_stop(GDisplay *g) {
		release_bus(g);
	}
#endif

// Not implemented yet.
#if 0 && GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDisplay *g) {
		switch(g->p.x) {
		case GDISP_CONTROL_POWER:
			if (g->g.Powermode == (powermode_t)g->p.ptr)
				return;
			switch((powermode_t)g->p.ptr) {
				case powerOff:
					acquire_bus(g);
					write_index(g, SSD1963_EXIT_SLEEP_MODE); // leave sleep mode
					gfxSleepMilliseconds(5);
					write_index(g, SSD1963_SET_DISPLAY_OFF);
					write_index(g, SSD1963_SET_DEEP_SLEEP); // enter deep sleep mode
					release_bus(g);
					break;
				case powerOn:
					acquire_bus(g);
					read_reg(0x0000); gfxSleepMilliseconds(5); // 2x Dummy reads to wake up from deep sleep
					read_reg(0x0000); gfxSleepMilliseconds(5);
					write_index(g, SSD1963_SET_DISPLAY_ON);
					release_bus(g);
					break;
				case powerSleep:
					acquire_bus(g);
					write_index(g, SSD1963_SET_DISPLAY_OFF);
					write_index(g, SSD1963_ENTER_SLEEP_MODE); // enter sleep mode
					release_bus(g);
					break;
				default:
					return;
			}
			g->g.Powermode = (powermode_t)g->p.ptr;
			return;

		case GDISP_CONTROL_ORIENTATION:
			if (g->g.Orientation == (orientation_t)g->p.ptr)
				return;
			case GDISP_ROTATE_0:
				acquire_bus(g);
				/* 	Code here */
				release_bus(g);
				g->g.Height = ((LCD_Parameters *)g->priv)->height;
				g->g.Width = ((LCD_Parameters *)g->priv)->width;
				break;
			case GDISP_ROTATE_90:
				acquire_bus(g);
				/* 	Code here */
				release_bus(g);
				g->g.Height = ((LCD_Parameters *)g->priv)->width;
				g->g.Width = ((LCD_Parameters *)g->priv)->height;
				break;
			case GDISP_ROTATE_180:
				acquire_bus(g);
				/* 	Code here */
				release_bus(g);
				g->g.Height = ((LCD_Parameters *)g->priv)->height;
				g->g.Width = ((LCD_Parameters *)g->priv)->width;
				break;
			case GDISP_ROTATE_270:
				acquire_bus(g);
				/* 	Code here */
				release_bus(g);
				g->g.Height = ((LCD_Parameters *)g->priv)->width;
				g->g.Width = ((LCD_Parameters *)g->priv)->height;
				break;
			default:
				return;
			}
		g->g.Orientation = (orientation_t)g->p.ptr;
			return;

		case GDISP_CONTROL_BACKLIGHT:
			if ((unsigned)g->p.ptr > 100)
				g->p.ptr = (void *)100;
			set_backlight(g, (unsigned)g->p.ptr);
			g->g.Backlight = (unsigned)g->p.ptr;
			return;

		//case GDISP_CONTROL_CONTRAST:
		default:
			return;
		}
	}
#endif

#endif /* GFX_USE_GDISP */
