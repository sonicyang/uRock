/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/class_gwin.h
 * @brief   GWIN Graphic window subsystem header file.
 *
 * @defgroup Internal Internal
 * @ingroup GWIN
 *
 * @note	These definitions are normally not used by an application program. They are useful
 * 			only if you want to create your own custom GWIN window or widget.
 * @note	To access these definitions you must include "gwin/class_gwin.h" in your source file.
 *
 * @{
 */
#ifndef _CLASS_GWIN_H
#define _CLASS_GWIN_H

#if GFX_USE_GWIN || defined(__DOXYGEN__)

/**
 * @brief	The predefined flags for a Window
 * @{
 */
#define GWIN_FLG_DYNAMIC				0x0001			// @< The GWIN structure is allocated
#define GWIN_FLG_VISIBLE				0x0002			// @< The window is visible
#define GWIN_FLG_MINIMIZED				0x0004			// @< The window is minimized
#define GWIN_FLG_MAXIMIZED				0x0008			// @< The window is maximized
#define GWIN_FLG_ENABLED				0x0010			// @< The window is enabled
#define GWIN_FLG_WIDGET					0x0020			// @< This is a widget
#define GWIN_FLG_ALLOCTXT				0x0040			// @< The widget text is allocated
#define GWIN_FLG_MOUSECAPTURE			0x0080			// @< The widget has captured the mouse
#define GWIN_FIRST_WM_FLAG				0x0100			// @< 4 bits free for the window manager to use
#define GWIN_FIRST_CONTROL_FLAG			0x1000			// @< 4 bits free for Windows and Widgets to use
/* @} */

/**
 * @brief	The Virtual Method Table for a GWIN window
 * @{
 */
typedef struct gwinVMT {
	const char *		classname;						// @< The GWIN classname (mandatory)
	size_t				size;							// @< The size of the class object
	void (*Destroy)		(GWindowObject *gh);			// @< The GWIN destroy function (optional)
	void (*Redraw)		(GWindowObject *gh);			// @< The GWIN redraw routine (optional)
	void (*AfterClear)	(GWindowObject *gh);			// @< The GWIN after-clear function (optional)
} gwinVMT;
/* @} */

#if GWIN_NEED_WIDGET || defined(__DOXYGEN__)

	/**
	 * @brief	An toggle/dial instance is not being used
	 */
	#define GWIDGET_NO_INSTANCE		((uint16_t)-1)

	/**
	 * @brief	The source handle that widgets use when sending events
	 */
	#define GWIDGET_SOURCE			((GSourceHandle)(void *)_gwidgetCreate)

	/**
	 * @brief	The Virtual Method Table for a widget
	 * @note	A widget must have a destroy function. Either use @p _gwidgetDestroy() or use your own function
	 * 			which internally calls @p _gwidgetDestroy().
	 * @note	A widget must have a redraw function. Use @p _gwidgetRedraw().
	 * @note	If toggleroles != 0, ToggleAssign(), ToggleGet() and one or both of ToggleOff() and ToggleOn() must be specified.
	 * @note	If dialroles != 0, DialAssign(), DialGet() and DialMove() must be specified.
	 * @{
	 */
	typedef struct gwidgetVMT {
		struct gwinVMT				g;														// @< This is still a GWIN
		void (*DefaultDraw)			(GWidgetObject *gw, void *param);						// @< The default drawing routine (mandatory)
		#if GINPUT_NEED_MOUSE
			struct {
				void (*MouseDown)		(GWidgetObject *gw, coord_t x, coord_t y);				// @< Process mouse down events (optional)
				void (*MouseUp)			(GWidgetObject *gw, coord_t x, coord_t y);				// @< Process mouse up events (optional)
				void (*MouseMove)		(GWidgetObject *gw, coord_t x, coord_t y);				// @< Process mouse move events (optional)
			};
		#endif
		#if GINPUT_NEED_TOGGLE
			struct {
				uint16_t				toggleroles;											// @< The roles supported for toggles (0->toggleroles-1)
				void (*ToggleAssign)	(GWidgetObject *gw, uint16_t role, uint16_t instance);	// @< Assign a toggle to a role (optional)
				uint16_t (*ToggleGet)	(GWidgetObject *gw, uint16_t role);						// @< Return the instance for a particular role (optional)
				void (*ToggleOff)		(GWidgetObject *gw, uint16_t role);						// @< Process toggle off events (optional)
				void (*ToggleOn)		(GWidgetObject *gw, uint16_t role);						// @< Process toggle on events (optional)
			};
		#endif
		#if GINPUT_NEED_TOGGLE
			struct {
				uint16_t				dialroles;												// @< The roles supported for dials (0->dialroles-1)
				void (*DialAssign)		(GWidgetObject *gw, uint16_t role, uint16_t instance);	// @< Test the role and save the dial instance handle (optional)
				uint16_t (*DialGet)		(GWidgetObject *gw, uint16_t role);						// @< Return the instance for a particular role (optional)
				void (*DialMove)		(GWidgetObject *gw, uint16_t role, uint16_t value, uint16_t max);	// @< Process dial move events (optional)
			};
		#endif
	} gwidgetVMT;
	/* @} */
#endif

// These flags are needed whether or not we are running a window manager.
/**
 * @brief	Flags for redrawing after a visibility change
 * @{
 */
#define GWIN_WMFLG_PRESERVE			0x0001						// @< Preserve whatever existing contents possible if a window can't redraw
#define GWIN_WMFLG_NOBGCLEAR		0x0002						// @< Don't clear the area if the window is not visible
#define GWIN_WMFLG_NOZORDER			0x0004						// @< Don't redraw higher z-order windows that overlap
/* @} */

#if GWIN_NEED_WINDOWMANAGER || defined(__DOXYGEN__)
	#if 1				// When we know that wmq is the first element of the GWindowObject structure
		#define QItem2GWindow(qi)		((GHandle)qi)
	#else
		#define QItem2GWindow(qi)		((GHandle)(((char *)(qi)) - (size_t)(&(((GWindowObject *)0)->wmq))))
	#endif

	// @note	There is only ever one instance of each GWindowManager type
	typedef struct GWindowManager {
		const struct gwmVMT	*vmt;
	} GWindowManager;

	/**
	 * @brief	The Virtual Method Table for a window manager
	 * @{
	 */
	typedef struct gwmVMT {
		void (*Init)		(void);									// @< The window manager has just been set as the current window manager
		void (*DeInit)		(void);									// @< The window manager has just been removed as the current window manager
		bool_t (*Add)		(GHandle gh, const GWindowInit *pInit);	// @< A window has been added
		void (*Delete)		(GHandle gh);							// @< A window has been deleted
		void (*Redraw)		(GHandle gh, int visflags);				// @< A window needs to be redraw (or undrawn)
		void (*Redim)		(GHandle gh, coord_t x, coord_t y, coord_t w, coord_t h);	// @< A window wants to be moved or resized
		void (*Raise)		(GHandle gh);							// @< A window wants to be on top
		void (*MinMax)		(GHandle gh, GWindowMinMax minmax);		// @< A window wants to be minimized/maximised
	} gwmVMT;
	/* @} */

	/**
	 * @brief	The list of all windows in the system
	 */
	extern gfxQueueASync	_GWINList;

	/**
	 * @brief	The current window manager
	 */
	extern GWindowManager * _GWINwm;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief	Initialise (and allocate if necessary) the base GWIN object
 *
 * @param[in]	g		The GDisplay to use for this window
 * @param[in]	pgw		The GWindowObject structure. If NULL one is allocated from the heap
 * @param[in]	pInit	The user initialization parameters
 * @param[in]	vmt		The virtual method table for the GWIN object
 * @param[in]	flags	The default flags to use
 *
 * @return	The GHandle of the created window
 *
 * @notapi
 */
GHandle _gwindowCreate(GDisplay *g, GWindowObject *pgw, const GWindowInit *pInit, const gwinVMT *vmt, uint32_t flags);

#if GWIN_NEED_WIDGET || defined(__DOXYGEN__)
	/**
	 * @brief	Initialise (and allocate if necessary) the base Widget object
	 *
	 * @param[in]	g		The GDisplay to display this window on
	 * @param[in]	pgw		The GWidgetObject structure. If NULL one is allocated from the heap
	 * @param[in]	pInit	The user initialization parameters
	 * @param[in]	vmt		The virtual method table for the Widget object
	 *
	 * @return	The GHandle of the created widget
 	 *
	 * @notapi
	 */
	GHandle _gwidgetCreate(GDisplay *g, GWidgetObject *pgw, const GWidgetInit *pInit, const gwidgetVMT *vmt);

	/**
	 * @brief	Destroy the Widget object
	 *
	 * @param[in]	gh		The widget to destroy
	 *
	 * @notapi
	 */
	void _gwidgetDestroy(GHandle gh);

	/**
	 * @brief	Redraw the Widget object
	 *
	 * @param[in]	gh		The widget to redraw
	 *
	 * @notapi
	 */
	void _gwidgetRedraw(GHandle gh);
#endif

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_GWIN */

#endif /* _CLASS_GWIN_H */
/** @} */
