/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if (GFX_USE_GINPUT && GINPUT_NEED_MOUSE) /*|| defined(__DOXYGEN__)*/

#include "src/ginput/driver_mouse.h"

#include "drivers/ginput/touch/FT5x06/ft5x06.h"

// include board abstraction 
#include "ginput_lld_mouse_board.h"

static coord_t x, y, z;
static uint8_t touched;

void ginput_lld_mouse_init(void) {
	init_board();

	// Init default values. (From NHD-3.5-320240MF-ATXL-CTP-1 datasheet)
	// Valid touching detect threshold
	write_reg(FT5x06_ID_G_THGROUP, 1, 0x16);

	// valid touching peak detect threshold
	write_reg(FT5x06_ID_G_THPEAK, 1, 0x3C);

	// Touch focus threshold
	write_reg(FT5x06_ID_G_THCAL, 1, 0xE9);

	// threshold when there is surface water
	write_reg(FT5x06_ID_G_THWATER, 1, 0x01);

	// threshold of temperature compensation
	write_reg(FT5x06_ID_G_THTEMP, 1, 0x01);

	// Touch difference threshold
	write_reg(FT5x06_ID_G_THDIFF, 1, 0xA0);

	// Delay to enter 'Monitor' status (s)
	write_reg(FT5x06_ID_G_TIME_ENTER_MONITOR, 1, 0x0A);

	// Period of 'Active' status (ms)
	write_reg(FT5x06_ID_G_PERIODACTIVE, 1, 0x06);

	// Timer to enter ÔidleÕ when in 'Monitor' (ms)
	write_reg(FT5x06_ID_G_PERIODMONITOR, 1, 0x28);
}

void ginput_lld_mouse_get_reading(MouseReading *pt) {
	// Poll to get the touched status
	uint8_t		last_touched;
	
	last_touched = touched;
	touched = (uint8_t)read_reg(FT5x06_TOUCH_POINTS, 1) & 0x07;

	// If not touched, return the previous results
	if (touched == 0) {
		pt->x = x;
		pt->y = y;
		pt->z = 0;
		pt->buttons = 0;
		return;
	}

	/* Get the X, Y, Z values */
	x = (coord_t)(read_reg(FT5x06_TOUCH1_XH, 2) & 0x0fff);
	y = (coord_t)read_reg(FT5x06_TOUCH1_YH, 2);
	z = 100;

	// Rescale X,Y,Z - X & Y don't need scaling when you are using calibration!
#if !GINPUT_MOUSE_NEED_CALIBRATION
	x = gdispGetWidth() - x / (4096/gdispGetWidth());
	y = y / (4096/gdispGetHeight());
#endif
	
	// Return the results. ADC gives values from 0 to 2^12 (4096)
	pt->x = x;
	pt->y = y;
	pt->z = z;
	pt->buttons = GINPUT_TOUCH_PRESSED;
}

#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */
