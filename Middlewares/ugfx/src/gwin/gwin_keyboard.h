/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/gwin_button.h
 * @brief   GWIN Graphic window subsystem header file.
 *
 * @defgroup Button Button
 * @ingroup Widgets
 *
 * @details		GWIN allows it to easily create buttons with different styles
 *				and check for different meta states such as: PRESSED, CLICKED,
 *				RELEASED etc.
 *
 * @pre			GFX_USE_GWIN must be set to TRUE in your gfxconf.h
 * @pre			GWIN_NEED_BUTTON must be set to TRUE in your gfxconf.h
 * @{
 */

#ifndef _GWIN_KEYBOARD_H
#define _GWIN_KEYBOARD_H

/* This file is included within "src/gwin/gwin_widget.h" */

/**
 * @brief	The Event Type for a Button Event
 */
#define GEVENT_GWIN_KEYBOARD		(GEVENT_GWIN_CTRL_FIRST+6)

/**
 * @brief	A Keyboard Event
 * @note	There are currently no GEventGWinButton listening flags - use 0 as the flags to @p gwinAttachListener()
 */
typedef GEventGWin		GEventGWinKeyboard;

struct keyinfo {
};

/**
 * @brief	The keyboard widget structure
 * @note	Do not use the members directly - treat it as a black-box.
 */
typedef struct GKeyboardObject {
	GWidgetObject			w;
	const struct GVKeyTable	*keytable;
	const char				**keyset;
	coord_t					keyx, keyy;
	coord_t					keycx, keycy;
	uint8_t					lastkeyrow, lastkeycol;
	uint8_t					keyrow, keycol;
	uint32_t				key;
} GKeyboardObject;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Create a keyboard widget.
 * @return  NULL if there is no resultant drawing area, otherwise a window handle.
 *
 * @param[in] g			The GDisplay to display this window on
 * @param[in] gb		The GKeyboardObject structure to initialise. If this is NULL the structure is dynamically allocated.
 * @param[in] pInit		The initialisation parameters
 *
 * @note				The drawing color and the background color get set to the current defaults. If you haven't called
 * 						@p gwinSetDefaultColor() or @p gwinSetDefaultBgColor() then these are White and Black respectively.
 * @note				The font gets set to the current default font. If you haven't called @p gwinSetDefaultFont() then there
 * 						is no default font and text drawing operations will no nothing.
 * @note				A keyboard remembers its normal drawing state. If there is a window manager then it is automatically
 * 						redrawn if the window is moved or its visibility state is changed.
 * @note				A keyboard supports mouse input.
 *
 * @api
 */	
GHandle gwinGKeyboardCreate(GDisplay *g, GKeyboardObject *gb, const GWidgetInit *pInit);
#define gwinKeyboardCreate(gb, pInit)			gwinGKeyboardCreate(GDISP, gb, pInit)

/**
 * @brief   Get the keyboard event source for a GWIN virtual keyboard
 * @return	The event source handle or NULL if this is not a virtual keyboard
 *
 * @param[in] gh		The GWIN virtual keyboard
 *
 * @note	Normal GINPUT Keyboard events are returned by this event source.
 */
GSourceHandle gwinKeyboardGetEventSource(GHandle gh);

/**
 * @brief   Set the layout for the virtual keyboard
 *
 * @param[in] gh		The GWIN virtual keyboard
 * @param[in] layout	The keyboard layout to use (described by gwin_keyboard_layout.h)
 *
 * @note	Changing the layout resets the keyboard to key set 0 of the keyboard and cancels any
 * 			pending shifts.
 */
void gwinKeyboardSetLayout(GHandle gh, struct GVKeyTable *layout);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _GWIN_KEYBOARD_H */
/** @} */
