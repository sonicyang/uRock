/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/gwin_frame.h
 * @brief   GWIN Graphic window subsystem header file.
 *
 * @defgroup Frame Frame
 * @ingroup Containers
 *
 * @details		A frame is a rectangular window that can have optional border as well as buttons to
 *				close, maximize and minimize it. The main purpose of this widget is to contain children. 	
 *
 * @pre			GFX_USE_GWIN must be set to TRUE in your gfxconf.h
 * @pre			GWIN_NEED_FRAME must be set to TRUE in your gfxconf.h
 * @{
 */

#ifndef _GWIN_FRAME_H
#define _GWIN_FRAME_H

/* This file is included from src/gwin/gwin_container.h */

/**
 * @brief	 Flags for gwinFrameCreate()
 * @{
 */
#define GWIN_FRAME_BORDER			0x00000000		// Deprecated. A border is always shown with a frame window now.
#define GWIN_FRAME_CLOSE_BTN		0x00000001
#define GWIN_FRAME_MINMAX_BTN		0x00000002
/** @} */

typedef GContainerObject GFrameObject;

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief				Create a frame widget
	 *
	 * @details				This widget provides a window like we know it from desktop systems.
	 *
	 * @param[in] g			The GDisplay to display this window on
	 * @param[in] fo		The GFrameObject structure to initialize. If this is NULL the structure is dynamically allocated.
	 * @param[in] pInit		The initialization parameters
	 * @param[in] flags		Some flags, see notes.
	 *
	 * @note				Possible flags are: GWIN_FRAME_CLOSE_BTN, GWIN_FRAME_MINMAX_BTN.
	 * @note				These frame buttons are processed internally. The close button will invoke a gwinDestroy() which will
	 *						destroy the window itself and EVERY child it contains (also children of children).
	 *
	 * @return				NULL if there is no resulting widget. A valid GHandle otherwise.
	 *
	 * @api
	 */
	GHandle gwinGFrameCreate(GDisplay *g, GFrameObject *fo, GWidgetInit *pInit, uint32_t flags);
	#define gwinFrameCreate(fo, pInit, flags)	gwinGFrameCreate(GDISP, fo, pInit, flags);

	/**
	 * @brief				The custom draw routines for a frame window
	 * @details				These function may be passed to @p gwinSetCustomDraw() to get different frame drawing styles
	 *
	 * @param[in] gw		The widget object (in this case a frame)
	 * @param[in] param		A parameter passed in from the user
	 *
	 * @note				In your own custom drawing function you may optionally call these
	 * 						standard functions and then draw your extra details on top.
	 *
	 * @note				gwinFrameDraw_Std() will fill the client area with the background color.<br/>
	 * 						gwinFrameDraw_Transparent() will not fill the client area at all.<br/>
	 * 						gwinFrameDraw_Image() will tile the image throughout the client area.<br/>
	 * 						All these drawing functions draw the frame itself the same way.
	 *
	 * @note				The standard functions below ignore the param parameter except for @p gwinFrameDraw_Image().
	 * @note				The image custom draw function  @p gwinFrameDraw_Image() uses param to pass in the gdispImage pointer.
	 * 						The image must be already opened before calling  @p gwinSetCustomDraw().
	 *
	 * @api
	 * @{
	 */
	void gwinFrameDraw_Std(GWidgetObject *gw, void *param);
	void gwinFrameDraw_Transparent(GWidgetObject *gw, void *param);
	void gwinFrameDraw_Image(GWidgetObject *gw, void *param);
	/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _GWIN_FRAME_H */
/** @} */

