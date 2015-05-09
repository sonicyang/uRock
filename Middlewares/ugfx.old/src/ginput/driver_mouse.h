/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/driver_mouse.h
 * @brief   GINPUT LLD header file for mouse/touch drivers.
 *
 * @defgroup Mouse Mouse
 * @ingroup GINPUT
 * @{
 */

#ifndef _LLD_GINPUT_MOUSE_H
#define _LLD_GINPUT_MOUSE_H

#if GINPUT_NEED_MOUSE || defined(__DOXYGEN__)

#include "ginput_lld_mouse_config.h"

// GEVENT_MOUSE or GEVENT_TOUCH - What type of device is this.
#ifndef GINPUT_MOUSE_EVENT_TYPE
	#define GINPUT_MOUSE_EVENT_TYPE					GEVENT_MOUSE
#endif

// TRUE/FALSE - Does the mouse/touch driver require calibration?
#ifndef GINPUT_MOUSE_NEED_CALIBRATION
	#define GINPUT_MOUSE_NEED_CALIBRATION			FALSE
#endif

// TRUE/FALSE - Should the calibration happen at the extremes of the panel?
#ifndef GINPUT_MOUSE_CALIBRATE_EXTREMES
	#define GINPUT_MOUSE_CALIBRATE_EXTREMES			FALSE
#endif

// TRUE/FALSE	- Can the mouse/touch driver itself save calibration data?
#ifndef GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE
	#define GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE	FALSE
#endif

// n or -1		- n means to test calibration result (+/- pixels), -1 means not to.
#ifndef GINPUT_MOUSE_MAX_CALIBRATION_ERROR
	#define GINPUT_MOUSE_MAX_CALIBRATION_ERROR		-1
#endif

// n			- How many times to read (and average) per poll
#ifndef GINPUT_MOUSE_READ_CYCLES
	#define GINPUT_MOUSE_READ_CYCLES				1
#endif

// n			 - Millisecs between poll's
#ifndef GINPUT_MOUSE_POLL_PERIOD
	#define GINPUT_MOUSE_POLL_PERIOD				25
#endif

// n			- Movement allowed without discarding the CLICK or CLICKCXT event (+/- pixels)
#ifndef GINPUT_MOUSE_MAX_CLICK_JITTER
	#define GINPUT_MOUSE_MAX_CLICK_JITTER			1
#endif

// n			- Movement allowed without discarding the MOVE event (+/- pixels)
#ifndef GINPUT_MOUSE_MAX_MOVE_JITTER
	#define GINPUT_MOUSE_MAX_MOVE_JITTER			0
#endif

// ms			- Millisecs seperating a CLICK from a CXTCLICK
#ifndef GINPUT_MOUSE_CLICK_TIME
	#define GINPUT_MOUSE_CLICK_TIME					700
#endif

// true/false	- Whether the mouse driver internally handles screen rotation
#ifndef GINPUT_MOUSE_NO_ROTATION
	#define GINPUT_MOUSE_NO_ROTATION				FALSE
#endif

typedef struct MouseReading_t {
	coord_t		x, y, z;
	uint16_t	buttons;
	} MouseReading;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief   Initialise the mouse/touch.
	 *
	 * @notapi
	 */
	void ginput_lld_mouse_init(void);

	/**
	 * @brief   Read the mouse/touch position.
	 *
	 * @param[in] pt	A pointer to the structure to fill
	 *
	 * @note			For drivers that don't support returning a position
	 *					when the touch is up (most touch devices), it should
	 *					return the previous position with the new Z value.
	 *					The z value is the pressure for those touch devices
	 *					that support it (-100 to 100 where > 0 is touched)
	 *					or, 0 or 100 for those drivers that don't.
	 *
	 * @notapi
	 */
	void ginput_lld_mouse_get_reading(MouseReading *pt);

	#if GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE
		/**
		 * @brief   Load calibration data from a storage area on the touch controller.
		 *
		 * @param[in] instance	The mouse instance number
		 *
		 * @note	The instance parameter is currently always 0 as we only support
		 * 			one mouse/touch device at a time.
		 * @note	This routine should only be provided if the driver has its own
		 * 			storage area where calibration data can be stored. The drivers
		 * 			option.h file should define GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE = TRUE
		 * 			if it supports this.
		 *
		 * @notapi
		 */
		const char *ginput_lld_mouse_calibration_load(uint16_t instance);
		/**
		 * @brief   Save calibration data to a storage area on the touch controller.
		 *
		 * @param[in] instance	The mouse instance number
		 * @param[in] calbuf	The calibration data to be saved
		 * @param[in] sz		The size of the calibration data
		 *
		 * @note	The instance parameter is currently always 0 as we only support
		 * 			one mouse/touch device at a time.
		 * @note	This routine should only be provided if the driver has its own
		 * 			storage area where calibration data can be stored. The drivers
		 * 			option.h file should define GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE = TRUE
		 * 			if it supports this.
		 *
		 * @notapi
		 */
		void ginput_lld_mouse_calibration_save(uint16_t instance, const uint8_t *calbuf, size_t sz);
	#endif

	/**
	 * @brief	Wakeup the high level code so that it attempts another read
	 *
	 * @note	This routine is provided to low level drivers by the high level code
	 * @note	Particularly useful if GINPUT_MOUSE_POLL_PERIOD = TIME_INFINITE
	 *
	 * @notapi
	 */
	void ginputMouseWakeup(void);

	/**
	 * @brief	Wakeup the high level code so that it attempts another read
	 *
	 * @note	This routine is provided to low level drivers by the high level code
	 * @note	Particularly useful if GINPUT_MOUSE_POLL_PERIOD = TIME_INFINITE
	 *
	 * @iclass
	 * @notapi
	 */
	void ginputMouseWakeupI(void);

#ifdef __cplusplus
}
#endif

#endif /* GINPUT_NEED_MOUSE || GINPUT_NEED_TOUCH */

#endif /* _LLD_GINPUT_MOUSE_H */
/** @} */
