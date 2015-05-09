/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/mouse.h
 * @brief   GINPUT GFX User Input subsystem header file for mouse and touch.
 *
 * @defgroup Mouse Mouse
 * @ingroup GINPUT
 *
 * @details GINPUT allows it to easily interface touchscreens and mices to
 *			your application.
 *
 * @pre		GFX_USE_GINPUT must be set to TRUE in your gfxconf.h
 * @pre		GINPUT_NEED_MOUSE must be set to TRUE in your gfxconf.h
 * 
 * @{
 */

#ifndef _GINPUT_MOUSE_H
#define _GINPUT_MOUSE_H

#if GINPUT_NEED_MOUSE || defined(__DOXYGEN__)

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

typedef int16_t coord_t;

typedef struct GDisplay GDisplay;

/* This type definition is also used by touch */
typedef struct GEventMouse_t {
	GEventType		type;				// The type of this event (GEVENT_MOUSE or GEVENT_TOUCH)
	uint16_t		instance;			// The mouse/touch instance
	coord_t			x, y, z;			// The position of the mouse.
										//		- For touch devices, Z is the current pressure if supported (otherwise 0)
										//		- For mice, Z is the 3rd dimension if supported (otherwise 0)
	uint16_t		current_buttons;	// A bit is set if the button is down.
										//		- For touch only bit 0 is relevant
										//		- For mice the order of the buttons is (from 0 to n)  left, right, middle, any other buttons
										//		- Bit 15 being set indicates that an important mouse event has been missed.
		#define GINPUT_MOUSE_BTN_LEFT		0x0001
		#define GINPUT_MOUSE_BTN_RIGHT		0x0002
		#define GINPUT_MOUSE_BTN_MIDDLE		0x0004
		#define GINPUT_MOUSE_BTN_4			0x0008
		#define GINPUT_MISSED_MOUSE_EVENT	0x8000
		#define GINPUT_TOUCH_PRESSED		GINPUT_MOUSE_BTN_LEFT
	uint16_t		last_buttons;		// The value of current_buttons on the last event
	enum GMouseMeta_e {
		GMETA_NONE = 0,						// There is no meta event currently happening
		GMETA_MOUSE_DOWN = 1,				// Button 0 has just gone down
		GMETA_MOUSE_UP = 2,					// Button 0 has just gone up
		GMETA_MOUSE_CLICK = 4,				// Button 0 has just gone through a short down - up cycle
		GMETA_MOUSE_CXTCLICK = 8			// For mice - The right button has just been depressed
											// For touch - a long press has just occurred
		}				meta;
	GDisplay *			display;		// The display this mouse is currently associated with.
	} GEventMouse;

// Mouse/Touch Listen Flags - passed to geventAddSourceToListener()
#define GLISTEN_MOUSEMETA			0x0001			// Create events for meta events such as CLICK and CXTCLICK
#define GLISTEN_MOUSEDOWNMOVES		0x0002			// Creates mouse move events when the primary mouse button is down (touch is on the surface)
#define GLISTEN_MOUSEUPMOVES		0x0004			// Creates mouse move events when the primary mouse button is up (touch is off the surface - if the hardware allows).
#define	GLISTEN_MOUSENOFILTER		0x0008			// Don't filter out mouse moves where the position hasn't changed.
#define GLISTEN_TOUCHMETA			GLISTEN_MOUSEMETA
#define GLISTEN_TOUCHDOWNMOVES		GLISTEN_MOUSEDOWNMOVES
#define GLISTEN_TOUCHUPMOVES		GLISTEN_MOUSEUPMOVES
#define	GLISTEN_TOUCHNOFILTER		GLISTEN_MOUSENOFILTER

#define GINPUT_MOUSE_NUM_PORTS		1			// The total number of mouse/touch inputs supported

// Event types for the mouse ginput source
#define GEVENT_MOUSE		(GEVENT_GINPUT_FIRST+0)
#define GEVENT_TOUCH		(GEVENT_GINPUT_FIRST+1)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief	Creates an instance of a mouse and returns the Source handler
	 * @note	HACK: if the instance is 9999, it is treated as instance 0 except
	 * 			that no calibration will be performed!
	 *
	 * @param[in] instance		The ID of the mouse input instance (from 0 to 9999)
	 *
	 * @return		The source handle of the created instance
	 */
	GSourceHandle ginputGetMouse(uint16_t instance);
	
	/**
	 * @brief	Assign the display associated with the mouse
	 * @note	This only needs to be called if the mouse is associated with a display
	 * 			other than the current default display. It must be called before
	 * 			@p ginputGetMouse() if the new display is to be used during the calibration
	 * 			process. Other than calibration the display is used for range checking,
	 * 			and may also be used to display a mouse pointer.
	 *
	 * @param[in] instance		The ID of the mouse input instance
	 * @param[in] g				The GDisplay to which this mouse belongs
	 */
	void ginputSetMouseDisplay(uint16_t instance, GDisplay *g);

	/**
	 * @brief	Get the display currently associated with the mouse
	 * @return	A pointer to the display
	 *
	 * @param[in] instance		The ID of the mouse input instance
	 */
	GDisplay *ginputGetMouseDisplay(uint16_t instance);

	/**
	 * @brief	Get the current mouse position and button status
	 * @note	Unlinke a listener event, this status cannot record meta events such as
	 *			"CLICK".
	 *
	 * @param[in] instance	The ID of the mouse input instance
	 * @param[in] pmouse	The mouse event
	 *
	 * @return	FALSE on an error (eg. invalid instance)
	 */
	bool_t ginputGetMouseStatus(uint16_t instance, GEventMouse *pmouse);

	/**
	 * @brief	Performs a calibration
	 *
	 * @param[in] instance	The ID of the mouse input instance
	 *
	 * @return	FALSE if the driver dosen't support a calibration of if the handle is invalid
	 */
	bool_t ginputCalibrateMouse(uint16_t instance);

	/* Set the routines to save and fetch calibration data.
	 * This function should be called before first calling ginputGetMouse() for a particular instance
	 *	as the gdispGetMouse() routine may attempt to fetch calibration data and perform a startup calibration if there is no way to get it.
	 *	If this is called after gdispGetMouse() has been called and the driver requires calibration storage, it will immediately save the data is has already obtained.
	 * The 'requireFree' parameter indicates if the fetch buffer must be free()'d to deallocate the buffer provided by the Fetch routine.
	 */
	typedef void (*GMouseCalibrationSaveRoutine)(uint16_t instance, const uint8_t *calbuf, size_t sz);			// Save calibration data
	typedef const char * (*GMouseCalibrationLoadRoutine)(uint16_t instance);									// Load calibration data (returns NULL if not data saved)

	/**
	 * @brief	Set the routines to store and restore calibration data
	 *
	 * @details	This function should be called before first calling ginputGetMouse() for a particular instance
	 *			as the gdispGetMouse() routine may attempt to fetch calibration data and perform a startup calibration if there is no way to get it.
	 *			If this is called after gdispGetMouse() has been called and the driver requires calibration storage, it will immediately save the
	 *			data is has already obtained.
	 *
	 * @param[in] instance		The ID of the mouse input instance
	 * @param[in] fnsave		The routine to save the data
	 * @param[in] fnload		The routine to restore the data
	 * @param[in] requireFree	TRUE if the buffer returned by the load function must be freed by the mouse code.
	 */	
	void ginputSetMouseCalibrationRoutines(uint16_t instance, GMouseCalibrationSaveRoutine fnsave, GMouseCalibrationLoadRoutine fnload, bool_t requireFree);

	/**
	 * @brief	Test if a particular mouse/touch instance requires routines to save it's alibration data
	 * @note	Not implemented yet
	 *
	 * @param[in] instance		The ID of the mouse input instance
	 *
	 * @return	TRUE if needed
	 */
	bool_t ginputRequireMouseCalibrationStorage(uint16_t instance);
	
#ifdef __cplusplus
}
#endif

#endif /* GINPUT_NEED_MOUSE */

#endif /* _GINPUT_MOUSE_H */
/** @} */

