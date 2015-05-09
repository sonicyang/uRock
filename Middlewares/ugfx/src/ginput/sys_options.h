/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/sys_options.h
 * @brief   GINPUT sub-system options header file.
 *
 * @addtogroup GINPUT
 * @{
 */

#ifndef _GINPUT_OPTIONS_H
#define _GINPUT_OPTIONS_H

/**
 * @name    GINPUT Functionality to be included
 * @{
 */
	/**
	 * @brief   Should mouse/touch functions be included.
	 * @details	Defaults to FALSE
	 * @note	Also add the a mouse/touch hardware driver to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/ginput/touch/MCU/ginput_lld.mk
	 */
	#ifndef GINPUT_NEED_MOUSE
		#define GINPUT_NEED_MOUSE		FALSE
	#endif
	/**
	 * @brief   Should keyboard functions be included.
	 * @details	Defaults to FALSE
	 * @note	Also add the a keyboard hardware driver to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/ginput/keyboard/XXXX/ginput_lld.mk
	 */
	#ifndef GINPUT_NEED_KEYBOARD
		#define GINPUT_NEED_KEYBOARD	FALSE
	#endif
	/**
	 * @brief   Should hardware toggle/switch/button functions be included.
	 * @details	Defaults to FALSE
	 * @note	Also add the a toggle hardware driver to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/ginput/toggle/Pal/ginput_lld.mk
	 */
	#ifndef GINPUT_NEED_TOGGLE
		#define GINPUT_NEED_TOGGLE		FALSE
	#endif
	/**
	 * @brief   Should analog dial functions be included.
	 * @details	Defaults to FALSE
	 * @note	Also add the a dial hardware driver to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/ginput/dial/analog/ginput_lld.mk
	 */
	#ifndef GINPUT_NEED_DIAL
		#define GINPUT_NEED_DIAL		FALSE
	#endif
/**
 * @}
 *
 * @name    GINPUT Optional Sizing Parameters
 * @{
 */
/**
 * @}
 *
 * @name    GINPUT Optional Low Level Driver Defines
 * @{
 */
	/**
	 * @brief   Use a custom board definition for the mouse/touch driver even if a board definition exists.
	 * @details	Defaults to FALSE
	 * @details	If TRUE, add ginput_lld_mouse_board.h to your project directory and customise it.
	 * @note	Not all GINPUT mouse/touch low level drivers use board definition files.
	 */
	#ifndef GINPUT_MOUSE_USE_CUSTOM_BOARD
		#define GINPUT_MOUSE_USE_CUSTOM_BOARD		FALSE
	#endif
	/**
	 * @brief   Use a custom board definition for the keyboard driver even if a board definition exists.
	 * @details	Defaults to FALSE
	 * @details	If TRUE, add ginput_lld_keyboard_board.h to your project directory and customise it.
	 * @note	Not all GINPUT keyboard low level drivers use board definition files.
	 */
	#ifndef GINPUT_KEYBOARD_USE_CUSTOM_BOARD
		#define GINPUT_KEYBOARD_USE_CUSTOM_BOARD	FALSE
	#endif
	/**
	 * @brief   Use a custom board definition for the toggle driver even if a board definition exists.
	 * @details	Defaults to FALSE
	 * @details	If TRUE, add ginput_lld_toggle_board.h to your project directory and customise it.
	 * @note	Not all GINPUT toggle low level drivers use board definition files.
	 */
	#ifndef GINPUT_TOGGLE_USE_CUSTOM_BOARD
		#define GINPUT_TOGGLE_USE_CUSTOM_BOARD		FALSE
	#endif
	/**
	 * @brief   Use a custom board definition for the dial driver even if a board definition exists.
	 * @details	Defaults to FALSE
	 * @details	If TRUE, add ginput_lld_dial_board.h to your project directory and customise it.
	 * @note	Not all GINPUT dial low level drivers use board definition files.
	 */
	#ifndef GINPUT_DIAL_USE_CUSTOM_BOARD
		#define GINPUT_DIAL_USE_CUSTOM_BOARD		FALSE
	#endif
/** @} */

#endif /* _GINPUT_OPTIONS_H */
/** @} */
