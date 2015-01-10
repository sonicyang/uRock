/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/ginput/touch/STMPE811/ginput_lld_mouse_config.h
 * @brief   GINPUT LLD header file for mouse/touch driver.
 *
 * @defgroup Mouse Mouse
 * @ingroup GINPUT
 *
 * @{
 */

#ifndef _LLD_GINPUT_MOUSE_CONFIG_H
#define _LLD_GINPUT_MOUSE_CONFIG_H

#define GINPUT_MOUSE_EVENT_TYPE					GEVENT_TOUCH
#define GINPUT_MOUSE_NEED_CALIBRATION			TRUE
#define GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE	FALSE
#define GINPUT_MOUSE_MAX_CALIBRATION_ERROR		5
#define GINPUT_MOUSE_READ_CYCLES				1
#define GINPUT_MOUSE_POLL_PERIOD				25
#define GINPUT_MOUSE_MAX_CLICK_JITTER			10
#define GINPUT_MOUSE_MAX_MOVE_JITTER			5
#define GINPUT_MOUSE_CLICK_TIME					450

#endif /* _LLD_GINPUT_MOUSE_CONFIG_H */
/** @} */
