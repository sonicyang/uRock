/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/keyboard.h
 * @brief   GINPUT GFX User Input subsystem header file.
 *
 * @defgroup Keyboard Keyboard
 * @ingroup GINPUT
 * @{
 */

#ifndef _GINPUT_KEYBOARD_H
#define _GINPUT_KEYBOARD_H

#if GINPUT_NEED_KEYBOARD || defined(__DOXYGEN__)

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

#define GINPUT_KEYBOARD_NUM_PORTS		1			// The total number of keyboard inputs

// Event types for various ginput sources
#define GEVENT_KEYBOARD		(GEVENT_GINPUT_FIRST+2)

typedef struct GEventKeyboard_t {
	GEventType		type;				// The type of this event (GEVENT_KEYBOARD)
	uint16_t		instance;			// The keyboard instance
	char			c;					// The Ascii code for the current key press.
										//		The only possible values are 0(NUL), 8(BS), 9(TAB), 13(CR), 27(ESC), 32(SPACE) to 126(~), 127(DEL)
										//		0 indicates an extended only key.
	uint16_t		code;				// An extended keyboard code. Codes less than 128 match their ascii equivelent.
		#define GKEY_NULL		0
		#define GKEY_BACKSPACE	8
		#define GKEY_TAB		9
		#define GKEY_CR			13
		#define GKEY_ESC		27
		#define GKEY_SPACE		32
		#define GKEY_DEL		127
		#define GKEY_UP			0x0101
		#define GKEY_DOWN		0x0102
		#define GKEY_LEFT		0x0103
		#define GKEY_RIGHT		0x0104
		#define GKEY_HOME		0x0105
		#define GKEY_END		0x0106
		#define GKEY_PAGEUP		0x0107
		#define GKEY_PAGEDOWN	0x0108
		#define GKEY_INSERT		0x0109
		#define GKEY_DELETE		0x010A
		#define GKEY_SHIFT		0x0201
		#define GKEY_CNTRL		0x0202
		#define GKEY_ALT		0x0203
		#define GKEY_WINKEY		0x0204
		#define GKEY_RCLKEY		0x0205
		#define GKEY_FNKEY		0x0206
		#define GKEY_FN1		0x0301
		#define GKEY_FN2		0x0302
		#define GKEY_FN3		0x0303
		#define GKEY_FN4		0x0304
		#define GKEY_FN5		0x0305
		#define GKEY_FN6		0x0306
		#define GKEY_FN7		0x0307
		#define GKEY_FN8		0x0308
		#define GKEY_FN9		0x0309
		#define GKEY_FN10		0x030A
		#define GKEY_FN11		0x030B
		#define GKEY_FN12		0x030C
	uint16_t		current_buttons;		// A bit is set to indicate various meta status.
		#define GMETA_KEY_DOWN			0x0001
		#define GMETA_KEY_SHIFT			0x0002
		#define GMETA_KEY_CNTRL			0x0004
		#define GMETA_KEY_ALT			0x0008
		#define GMETA_KEY_WINKEY		0x0010
		#define GMETA_KEY_RCLKKEY		0x0020
		#define GMETA_KEY_FN			0x0040
		#define GMETA_KEY_MISSED_EVENT	0x8000
	uint16_t		last_buttons;			// The value of current_buttons on the last event
} GEventKeyboard;

// Keyboard Listen Flags - passed to geventAddSourceToListener()
#define GLISTEN_KEYREPEATS		0x0001			// Return key repeats (where the key is held down to get a repeat character)
#define GLISTEN_KEYCODES		0x0002			// Return all key presses including extended code key presses (not just ascii codes)
#define GLISTEN_KEYALL			0x0004			// Return keyup's, keydown's and everything in between (but not repeats unless GLISTEN_KEYREPEATS is set).
#define GLISTEN_KEYSINGLE		0x8000			// Return only when one particular extended code key is pressed or released. The particular extended code is OR'd into this value
												//		eg. (GLISTEN_KEYSINGLE | GKEY_CR)
												//		No other flags may be set with this flag.

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief	Create a keyboard input instance
	 *
	 * @param[in] instance	The ID of the keyboard input instance (from 0 to 9999)
	 *
	 * @return	The source handle of the created input instance
	 */
	GSourceHandle ginputGetKeyboard(uint16_t instance);
	
	/**
	 * @brief	Get the current keyboard status
	 *
	 * @param[in] instance	The ID of the keyboard input instance
	 * @param[in] pkeyboard	The keyboard event struct
	 *
	 * @return Returns FALSE on an error (eg invalid instance)
	 */
	bool_t ginputGetKeyboardStatus(uint16_t instance, GEventKeyboard *pkeyboard);

#ifdef __cplusplus
}
#endif

#endif /* GINPUT_NEED_KEYBOARD */

#endif /* _GINPUT_KEYBOARD_H */
/** @} */

