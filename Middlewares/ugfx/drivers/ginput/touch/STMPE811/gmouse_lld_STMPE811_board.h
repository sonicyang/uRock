/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

// Resolution and Accuracy Settings
#define GMOUSE_STMPE811_PEN_CALIBRATE_ERROR		8
#define GMOUSE_STMPE811_PEN_CLICK_ERROR			6
#define GMOUSE_STMPE811_PEN_MOVE_ERROR			4
#define GMOUSE_STMPE811_FINGER_CALIBRATE_ERROR	14
#define GMOUSE_STMPE811_FINGER_CLICK_ERROR		18
#define GMOUSE_STMPE811_FINGER_MOVE_ERROR		14

// How much extra data to allocate at the end of the GMouse structure for the board's use
#define GMOUSE_STMPE811_BOARD_DATA_SIZE			0

// Options - Leave these commented to make it user configurable in the gfxconf.h
//#define GMOUSE_STMPE811_READ_PRESSURE		FALSE
//#define GMOUSE_STMPE811_SELF_CALIBRATE	FALSE
//#define GMOUSE_STMPE811_TEST_MODE			FALSE

// If TRUE this board has the STMPE811 IRQ pin connected to a GPIO.
// Note: For tested hardware this is unreliable and should be set to FALSE until tested.
//			Symptoms are that mouse readings just appear to stop for a bit. Lifting the touch
//			and re-applying the touch cause readings to start again.
#define GMOUSE_STMPE811_GPIO_IRQPIN				TRUE

// If TRUE this is a really slow CPU and we should always clear the FIFO between reads.
#define GMOUSE_STMPE811_SLOW_CPU				FALSE


#endif /* _GINPUT_LLD_MOUSE_BOARD_H */
