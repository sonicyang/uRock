/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/gwin_slider.h
 * @brief   GWIN Graphic window subsystem header file.
 *
 * @defgroup Slider Slider
 * @ingroup Widgets
 *
 * @details		Create sliders with different styles
 *
 * @pre			GFX_USE_GWIN must be set to TRUE in your gfxconf.h
 * @pre			GWIN_NEED_SLIDER must be set to TRUE in your gfxconf.h
 * @{
 */

#ifndef _GWIN_SLIDER_H
#define _GWIN_SLIDER_H

/* This file is included within "src/gwin/gwin_widget.h" */

#define GEVENT_GWIN_SLIDER		(GEVENT_GWIN_CTRL_FIRST+1)

typedef struct GEventGWinSlider {
	GEventType		type;				// The type of this event (GEVENT_GWIN_SLIDER)
	GHandle			gwin;				// The slider that is returning results
	#if GWIN_WIDGET_TAGS
		WidgetTag	tag;				// The slider tag
	#endif
	int				position;

	uint8_t			action;
		#define GSLIDER_EVENT_SET		4		/* Slider position is set. This is the only event returned by default   */
		#define GSLIDER_EVENT_CANCEL	3		/* Slider position changing has been cancelled */
		#define GSLIDER_EVENT_START		2		/* Slider position has started changing */
		#define GSLIDER_EVENT_MOVE		1		/* Slider position has been moved */
} GEventGWinSlider;

// There are currently no GEventGWinSlider listening flags - use 0

// A slider window
typedef struct GSliderObject {
	GWidgetObject		w;
	#if GINPUT_NEED_TOGGLE
		uint16_t		t_dn;
		uint16_t		t_up;
	#endif
	#if GINPUT_NEED_DIAL
		uint16_t		dial;
	#endif
	coord_t				dpos;
	int					min;
	int					max;
	int					pos;
} GSliderObject;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Create a slider window.
 * @return  NULL if there is no resultant drawing area, otherwise a window handle.
 *
 * @param[in] g			The GDisplay to display this window on
 * @param[in] gb		The GSliderObject structure to initialise. If this is NULL the structure is dynamically allocated.
 * @param[in] pInit		The initialization parameters to use
 *
 * @note				The drawing color and the background color get set to the current defaults. If you haven't called
 * 						@p gwinSetDefaultColor() or @p gwinSetDefaultBgColor() then these are White and Black respectively.
 * @note				The font gets set to the current default font. If you haven't called @p gwinSetDefaultFont() then there
 * 						is no default font and text drawing operations will no nothing.
 * @note				A slider remembers its normal drawing state. If there is a window manager then it is automatically
 * 						redrawn if the window is moved or its visibility state is changed.
 * @note				The initial slider range is from 0 to 100 with an initial position of 0.
 * @note				A slider supports mouse, toggle and dial input.
 * @note				When assigning a toggle, only one toggle is supported per role. If you try to assign more than
 * 						one toggle to a role it will forget the previous toggle. Two roles are supported:
 * 						Role 0 = toggle for down, Role 1 = toggle for up.
 * @note				When assigning a dial, only one dial is supported. If you try to assign more than one dial
 * 						it will forget the previous dial. Only dial role 0 is supported.
 *
 * @api
 */	
GHandle gwinGSliderCreate(GDisplay *g, GSliderObject *gb, const GWidgetInit *pInit);
#define gwinSliderCreate(w, pInit)			gwinGSliderCreate(GDISP, w, pInit)

/**
 * @brief   Set the slider range.
 *
 * @param[in] gh		The window handle (must be a slider window)
 * @param[in] min		The minimum value
 * @param[in] max		The maximum value
 * @note				Sets the position to the minimum value.
 * @note				The slider is not automatically drawn. Call gwinSliderDraw() after changing the range.
 *
 * @api
 */
void gwinSliderSetRange(GHandle gh, int min, int max);

/**
 * @brief   Set the slider position.
 *
 * @param[in] gh		The window handle (must be a slider window)
 * @param[in] pos		The new position
 * @note				If the new position is outside the slider range then the position
 * 						is set to the closest end of the range.
 * @note				The slider is not automatically drawn. Call gwinSliderDraw() after changing the position.
 *
 * @api
 */
void gwinSliderSetPosition(GHandle gh, int pos);

/**
 * @brief   Get the current slider position.
 * @return	The slider position
 *
 * @param[in] gh		The window handle (must be a slider window)
 *
 * @note	The use of a listener to get the slider position is recommended if you
 * 			want continuous updates on the slider position.
 *
 * @api
 */
#define gwinSliderGetPosition(gh)		(((GSliderObject *)(gh))->pos)

/**
 * @brief   Should the slider send extended events.
 *
 * @param[in] gh		The window handle (must be a slider window)
 * @param[in] enabled	TRUE to enable extended events, FALSE to disable them
 *
 * @note	The slider by default will only send slider events with an action of GSLIDER_EVENT_SET.
 * 			This call can be used to enable other slider action's to be sent as events
 *
 * @api
 */
void gwinSliderSendExtendedEvents(GHandle gh, bool_t enabled);

/**
 * @brief	Some custom slider drawing routines
 * @details	These function may be passed to @p gwinSetCustomDraw() to get different slider drawing styles
 *
 * @param[in] gw			The widget (which must be a slider)
 * @param[in] param			A parameter passed in from the user
 *
 * @note				In your custom slider drawing function you may optionally call this
 * 						standard functions and then draw your extra details on top.
 * @note				The standard functions below ignore the param parameter except for @p gwinSliderDraw_Image().
 * @note				The image custom draw function  @p gwinSliderDraw_Image() uses param to pass in the gdispImage pointer.
 * 						The image must be already opened before calling  @p gwinSetCustomDraw(). The image is tiled to fill
 * 						the active area of the slider. The normal colors apply to the border and inactive area and the dividing line
 * 						between the active and inactive areas.
 * 						No checking is done to compare the dimensions of the slider to the size of the image.
 * 						Note text is drawn on top of the image.
 * @note				These custom drawing routines don't have to worry about setting clipping as the framework
 * 						sets clipping to the object window prior to calling these routines.
 *
 * @api
 * @{
 */
void gwinSliderDraw_Std(GWidgetObject *gw, void *param);
void gwinSliderDraw_Image(GWidgetObject *gw, void *param);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _GWIN_SLIDER_H */
/** @} */
