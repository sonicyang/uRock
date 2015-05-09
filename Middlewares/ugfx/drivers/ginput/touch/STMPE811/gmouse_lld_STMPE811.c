/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE

#define GMOUSE_DRIVER_VMT		GMOUSEVMT_STMPE811
#include "src/ginput/ginput_driver_mouse.h"

// Hardware definitions
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

// Get the hardware interface
#include "gmouse_lld_STMPE811_board.h"

// Extra settings for the users gfxconf.h file. See readme.txt
#ifndef GMOUSE_STMPE811_SELF_CALIBRATE
	#define GMOUSE_STMPE811_SELF_CALIBRATE	FALSE
#endif
#ifndef GMOUSE_STMPE811_READ_PRESSURE
	#define GMOUSE_STMPE811_READ_PRESSURE	FALSE
#endif
#ifndef GMOUSE_STMPE811_TEST_MODE
	#define GMOUSE_STMPE811_TEST_MODE		FALSE
#endif

/**
 * Notes:
 *
 * This chip has some problems which required careful coding to overcome.
 *
 * The interrupt pin seems to be unreliable, at least on some boards, so we at most
 * 	use the pin for filtering results to reduce cpu load.
 * 	The symptoms are that readings will just stop due to the irq not being asserted
 * 	even though there are items in the fifo. Another interrupt source such as a
 * 	touch transition will restart the irq.
 *
 * There is no fifo entry created when a touch up event occurs. We must therefore
 * 	generate a pseudo result on touch up. Fortunately the touch detection appears
 * 	reliable and so we turn off the driver GMOUSE_VFLG_POORUPDOWN setting. In practice
 * 	if touch is up we always return a pseudo event as this saves having to remember the
 * 	previous touch state.
 *
 * Z readings range from around 90 (fully touched) to around 150 (on the verge of non-touched).
 * Note the above is on the STM32F429i-Discovery board. Other boards may be different.
 * To be conservative we use 255 as touch off, anything else is a touch on.
 *
 * GMOUSE_STMPE811_TEST_MODE is designed to be used with the "touch_raw_readings" tool which shows
 * a steady stream of raw readings.
 *
 * Settings that may need tweaking on other hardware:
 * 		The settling times. We have set these conservatively at 1ms.
 * 		The reading window. We set this to 16 just to reduce noise. High-res panels may need a lower value.
 */
static bool_t MouseInit(GMouse* m, unsigned driverinstance) {
	BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
	return TRUE;
}

static bool_t read_xyz(GMouse* m, GMouseReading* pt)
{
	TS_StateTypeDef tps;

    BSP_TS_GetState(&tps);
	if (tps.TouchDetected) {
		pt->buttons = 1;
		pt->x = tps.X;
		pt->y = tps.Y;
		pt->z = tps.Z;
	}
	else {
		pt->buttons = 0;
		pt->z = 255; //Dick Ass touched determination based on Z, didn't I fking tell u that im not being touched?
	}

	return TRUE;
}

const GMouseVMT const GMOUSE_DRIVER_VMT[1] = {{
	{
		GDRIVER_TYPE_TOUCH,
		#if GMOUSE_STMPE811_SELF_CALIBRATE
			GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_ONLY_DOWN,
		#else
			GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_ONLY_DOWN | GMOUSE_VFLG_CALIBRATE | GMOUSE_VFLG_CAL_TEST,
		#endif
		sizeof(GMouse) + GMOUSE_STMPE811_BOARD_DATA_SIZE,
		_gmouseInitDriver,
		_gmousePostInitDriver,
		_gmouseDeInitDriver
	},
	0,				// z_max	- 0 indicates full touch
	255,			// z_min
	150,			// z_touchon
	255,			// z_touchoff
	{				// pen_jitter
		GMOUSE_STMPE811_PEN_CALIBRATE_ERROR,		// calibrate
		GMOUSE_STMPE811_PEN_CLICK_ERROR,			// click
		GMOUSE_STMPE811_PEN_MOVE_ERROR				// move
	},
	{				// finger_jitter
		GMOUSE_STMPE811_FINGER_CALIBRATE_ERROR,		// calibrate
		GMOUSE_STMPE811_FINGER_CLICK_ERROR,			// click
		GMOUSE_STMPE811_FINGER_MOVE_ERROR			// move
	},
	MouseInit, 		// init
	0,				// deinit
	read_xyz,		// get
	0,				// calsave
	0				// calload
}};

#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */

