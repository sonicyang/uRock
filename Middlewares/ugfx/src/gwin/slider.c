/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/slider.c
 * @brief   GWIN sub-system slider code.
 *
 * @defgroup Slider Slider
 * @ingroup GWIN
 *
 * @{
 */

#include "gfx.h"

#if (GFX_USE_GWIN && GWIN_NEED_SLIDER) || defined(__DOXYGEN__)

#include "src/gwin/class_gwin.h"

#ifndef GWIN_SLIDER_DEAD_BAND
	#define GWIN_SLIDER_DEAD_BAND	5
#endif

#ifndef GWIN_SLIDER_TOGGLE_INC
	#define GWIN_SLIDER_TOGGLE_INC	20			// How many toggles to go from minimum to maximum
#endif

// Send the slider event
static void SendSliderEvent(GWidgetObject *gw) {
	GSourceListener	*	psl;
	GEvent *			pe;
	#define pse			((GEventGWinSlider *)pe)

	// Trigger a GWIN Button Event
	psl = 0;
	while ((psl = geventGetSourceListener(GWIDGET_SOURCE, psl))) {
		if (!(pe = geventGetEventBuffer(psl)))
			continue;
		pse->type = GEVENT_GWIN_SLIDER;
		pse->slider = (GHandle)gw;
		pse->position = ((GSliderObject *)gw)->pos;
		geventSendEvent(psl);
	}

	#undef pse
}

// Reset the display position back to the value predicted by the saved slider position
static void ResetDisplayPos(GSliderObject *gsw) {
	if (gsw->w.g.width < gsw->w.g.height)
		gsw->dpos = gsw->w.g.height-1-((gsw->w.g.height-1)*(gsw->pos-gsw->min))/(gsw->max-gsw->min);
	else
		gsw->dpos = ((gsw->w.g.width-1)*(gsw->pos-gsw->min))/(gsw->max-gsw->min);
}

#if GINPUT_NEED_MOUSE
	// A mouse up event
	static void MouseUp(GWidgetObject *gw, coord_t x, coord_t y) {
		#define gsw		((GSliderObject *)gw)
		#define gh		((GHandle)gw)

		#if GWIN_BUTTON_LAZY_RELEASE
			// Clip to the slider
			if (x < 0) x = 0;
			else if (x >= gh->width) x = gh->width-1;
			if (y < 0) y = 0;
			else if (y >= gh->height) x = gh->height-1;
		#else
			// Are we over the slider?
			if (x < 0 || x >= gh->width || y < 0 || y >= gh->height) {
				// No - restore the slider
				ResetDisplayPos(gsw);
				_gwidgetRedraw(gh);
				return;
			}
		#endif

		// Set the new position
		if (gh->width < gh->height) {
			if (y > gh->height-GWIN_SLIDER_DEAD_BAND)
				gsw->pos = gsw->min;
			else if (y < GWIN_SLIDER_DEAD_BAND)
				gsw->pos = gsw->max;
			else
				gsw->pos = (uint16_t)((int32_t)(gh->height-1-y-GWIN_SLIDER_DEAD_BAND)*(gsw->max-gsw->min)/(gh->height-2*GWIN_SLIDER_DEAD_BAND) + gsw->min);
		} else {
			if (x > gh->width-GWIN_SLIDER_DEAD_BAND)
				gsw->pos = gsw->max;
			else if (x < GWIN_SLIDER_DEAD_BAND)
				gsw->pos = gsw->min;
			else
				gsw->pos = (uint16_t)((int32_t)(x-GWIN_SLIDER_DEAD_BAND)*(gsw->max-gsw->min)/(gh->width-2*GWIN_SLIDER_DEAD_BAND) + gsw->min);
		}

		ResetDisplayPos(gsw);
		_gwidgetRedraw(gh);

		// Generate the event
		SendSliderEvent(gw);
		#undef gh
		#undef gsw
	}

	// A mouse move (or mouse down) event
	static void MouseMove(GWidgetObject *gw, coord_t x, coord_t y) {
		#define gsw		((GSliderObject *)gw)

		// Determine the temporary display position (with range checking)
		if (gw->g.width < gw->g.height) {
			if (y < 0)
				gsw->dpos = 0;
			else if (y >= gw->g.height)
				gsw->dpos = gw->g.height-1;
			else
				gsw->dpos = y;
		} else {
			if (x < 0)
				gsw->dpos = 0;
			else if (x >= gw->g.width)
				gsw->dpos = gw->g.width-1;
			else
				gsw->dpos = x;
		}

		// Update the display
		_gwidgetRedraw(&gw->g);
		#undef gsw
	}
#endif

#if GINPUT_NEED_TOGGLE
	// A toggle on has occurred
	static void ToggleOn(GWidgetObject *gw, uint16_t role) {
		#define gsw		((GSliderObject *)gw)

		if (role) {
			gwinSliderSetPosition((GHandle)gw, gsw->pos+(gsw->max-gsw->min)/GWIN_SLIDER_TOGGLE_INC);
			SendSliderEvent(gw);
		} else {
			gwinSliderSetPosition((GHandle)gw, gsw->pos-(gsw->max-gsw->min)/GWIN_SLIDER_TOGGLE_INC);
			SendSliderEvent(gw);
		}
		#undef gsw
	}

	static void ToggleAssign(GWidgetObject *gw, uint16_t role, uint16_t instance) {
		if (role)
			((GSliderObject *)gw)->t_up = instance;
		else
			((GSliderObject *)gw)->t_dn = instance;
	}

	static uint16_t ToggleGet(GWidgetObject *gw, uint16_t role) {
		return role ? ((GSliderObject *)gw)->t_up : ((GSliderObject *)gw)->t_dn;
	}
#endif

#if GINPUT_NEED_DIAL
	// A dial move event
	static void DialMove(GWidgetObject *gw, uint16_t role, uint16_t value, uint16_t max) {
		#define gsw		((GSliderObject *)gw)
		(void)			role;

		// Set the new position
		gsw->pos = (uint16_t)((uint32_t)value*(gsw->max-gsw->min)/max + gsw->min);

		ResetDisplayPos(gsw);
		_gwidgetRedraw((GHandle)gw);

		// Generate the event
		SendSliderEvent(gw);
		#undef gsw
	}

	static void DialAssign(GWidgetObject *gw, uint16_t role, uint16_t instance) {
		(void) role;
		((GSliderObject *)gw)->dial = instance;
	}

	static uint16_t DialGet(GWidgetObject *gw, uint16_t role) {
		(void) role;
		return ((GSliderObject *)gw)->dial;
	}
#endif

// The slider VMT table
static const gwidgetVMT sliderVMT = {
	{
		"Slider",				// The classname
		sizeof(GSliderObject),	// The object size
		_gwidgetDestroy,		// The destroy routine
		_gwidgetRedraw,			// The redraw routine
		0,						// The after-clear routine
	},
	gwinSliderDraw_Std,			// The default drawing routine
	#if GINPUT_NEED_MOUSE
		{
			0,						// Process mouse down events (NOT USED)
			MouseUp,				// Process mouse up events
			MouseMove,				// Process mouse move events
		},
	#endif
	#if GINPUT_NEED_TOGGLE
		{
			2,						// 1 toggle role
			ToggleAssign,			// Assign Toggles
			ToggleGet,				// Get Toggles
			0,						// Process toggle off events (NOT USED)
			ToggleOn,				// Process toggle on events
		},
	#endif
	#if GINPUT_NEED_DIAL
		{
			1,						// 1 dial roles
			DialAssign,				// Assign Dials
			DialGet,				// Get Dials
			DialMove,				// Process dial move events
		},
	#endif
};

GHandle gwinGSliderCreate(GDisplay *g, GSliderObject *gs, const GWidgetInit *pInit) {
	if (!(gs = (GSliderObject *)_gwidgetCreate(g, &gs->w, pInit, &sliderVMT)))
		return 0;
	#if GINPUT_NEED_TOGGLE
		gs->t_dn = GWIDGET_NO_INSTANCE;
		gs->t_up = GWIDGET_NO_INSTANCE;
	#endif
	#if GINPUT_NEED_DIAL
		gs->dial = GWIDGET_NO_INSTANCE;
	#endif
	gs->min = 0;
	gs->max = 100;
	gs->pos = 0;
	ResetDisplayPos(gs);
	gwinSetVisible((GHandle)gs, pInit->g.show);
	return (GHandle)gs;
}

void gwinSliderSetRange(GHandle gh, int min, int max) {
	#define gsw		((GSliderObject *)gh)

	if (gh->vmt != (gwinVMT *)&sliderVMT)
		return;

	if (min == max)		// prevent divide by 0 errors.
		max++;
	gsw->min = min;
	gsw->max = max;
	gsw->pos = min;
	ResetDisplayPos(gsw);
	#undef gsw
}

void gwinSliderSetPosition(GHandle gh, int pos) {
	#define gsw		((GSliderObject *)gh)

	if (gh->vmt != (gwinVMT *)&sliderVMT)
		return;

	if (gsw->min <= gsw->max) {
		if (pos < gsw->min) gsw->pos = gsw->min;
		else if (pos > gsw->max) gsw->pos = gsw->max;
		else gsw->pos = pos;
	} else {
		if (pos > gsw->min) gsw->pos = gsw->min;
		else if (pos < gsw->max) gsw->pos = gsw->max;
		else gsw->pos = pos;
	}
	ResetDisplayPos(gsw);
	_gwidgetRedraw(gh);

	#undef gsw
}

/*----------------------------------------------------------
 * Custom Draw Routines
 *----------------------------------------------------------*/

void gwinSliderDraw_Std(GWidgetObject *gw, void *param) {
	#define gsw			((GSliderObject *)gw)
	const GColorSet *	pcol;
	(void)				param;

	if (gw->g.vmt != (gwinVMT *)&sliderVMT)
		return;

	if ((gw->g.flags & GWIN_FLG_ENABLED))
		pcol = &gw->pstyle->pressed;
	else
		pcol = &gw->pstyle->disabled;

	if (gw->g.width < gw->g.height) {			// Vertical slider
		if (gsw->dpos != gw->g.height-1)
			gdispGFillArea(gw->g.display, gw->g.x, gw->g.y+gsw->dpos, gw->g.width, gw->g.height - gsw->dpos, pcol->progress);	// Active Area
		if (gsw->dpos != 0)
			gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gsw->dpos, gw->pstyle->enabled.progress);			// Inactive area
		gdispGDrawBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, pcol->edge);								// Edge
		gdispGDrawLine(gw->g.display, gw->g.x, gw->g.y+gsw->dpos, gw->g.x+gw->g.width-1, gw->g.y+gsw->dpos, pcol->edge);	// Thumb
		if (gsw->dpos >= 2)
			gdispGDrawLine(gw->g.display, gw->g.x, gw->g.y+gsw->dpos-2, gw->g.x+gw->g.width-1, gw->g.y+gsw->dpos-2, pcol->edge);	// Thumb
		if (gsw->dpos <= gw->g.height-2)
			gdispGDrawLine(gw->g.display, gw->g.x, gw->g.y+gsw->dpos+2, gw->g.x+gw->g.width-1, gw->g.y+gsw->dpos+2, pcol->edge);	// Thumb

	// Horizontal slider
	} else {
		if (gsw->dpos != gw->g.width-1)
			gdispGFillArea(gw->g.display, gw->g.x+gsw->dpos, gw->g.y, gw->g.width-gsw->dpos, gw->g.height, gw->pstyle->enabled.progress);	// Inactive area
		if (gsw->dpos != 0)
			gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gsw->dpos, gw->g.height, pcol->progress);	// Active Area
		gdispGDrawBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, pcol->edge);								// Edge
		gdispGDrawLine(gw->g.display, gw->g.x+gsw->dpos, gw->g.y, gw->g.x+gsw->dpos, gw->g.y+gw->g.height-1, pcol->edge);	// Thumb
		if (gsw->dpos >= 2)
			gdispGDrawLine(gw->g.display, gw->g.x+gsw->dpos-2, gw->g.y, gw->g.x+gsw->dpos-2, gw->g.y+gw->g.height-1, pcol->edge);	// Thumb
		if (gsw->dpos <= gw->g.width-2)
			gdispGDrawLine(gw->g.display, gw->g.x+gsw->dpos+2, gw->g.y, gw->g.x+gsw->dpos+2, gw->g.y+gw->g.height-1, pcol->edge);	// Thumb
	}
	gdispGDrawStringBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, gw->text, gw->g.font, pcol->text, justifyCenter);

	#undef gsw
}

#if GDISP_NEED_IMAGE
void gwinSliderDraw_Image(GWidgetObject *gw, void *param) {
	#define gsw			((GSliderObject *)gw)
	#define gi			((gdispImage *)param)
	const GColorSet *	pcol;
	coord_t				z, v;

	if (gw->g.vmt != (gwinVMT *)&sliderVMT)
		return;

	if ((gw->g.flags & GWIN_FLG_ENABLED))
		pcol = &gw->pstyle->pressed;
	else
		pcol = &gw->pstyle->disabled;

	if (gw->g.width < gw->g.height) {			// Vertical slider
		if (gsw->dpos != 0)							// The unfilled area
			gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gsw->dpos, gw->pstyle->enabled.progress);	// Inactive area
		if (gsw->dpos != gw->g.height-1) {			// The filled area
			for(z=gw->g.height, v=gi->height; z > gsw->dpos;) {
				z -= v;
				if (z < gsw->dpos) {
					v -= gsw->dpos - z;
					z = gsw->dpos;
				}
				gdispGImageDraw(gw->g.display, gi, gw->g.x, gw->g.y+z, gw->g.width, v, 0, gi->height-v);
			}
		}
		gdispGDrawBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, pcol->edge);								// Edge
		gdispGDrawLine(gw->g.display, gw->g.x, gw->g.y+gsw->dpos, gw->g.x+gw->g.width-1, gw->g.y+gsw->dpos, pcol->edge);	// Thumb

	// Horizontal slider
	} else {
		if (gsw->dpos != gw->g.width-1)				// The unfilled area
			gdispGFillArea(gw->g.display, gw->g.x+gsw->dpos, gw->g.y, gw->g.width-gsw->dpos, gw->g.height, gw->pstyle->enabled.progress);	// Inactive area
		if (gsw->dpos != 0) {						// The filled area
			for(z=0, v=gi->width; z < gsw->dpos; z += v) {
				if (z+v > gsw->dpos)
					v -= z+v - gsw->dpos;
				gdispGImageDraw(gw->g.display, gi, gw->g.x+z, gw->g.y, v, gw->g.height, 0, 0);
			}
		}
		gdispGDrawBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, pcol->edge);								// Edge
		gdispGDrawLine(gw->g.display, gw->g.x+gsw->dpos, gw->g.y, gw->g.x+gsw->dpos, gw->g.y+gw->g.height-1, pcol->edge);	// Thumb
	}
	gdispGDrawStringBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, gw->text, gw->g.font, pcol->text, justifyCenter);

	#undef gsw
}
#endif /* GDISP_NEED_IMAGE */

#endif /* GFX_USE_GWIN && GWIN_NEED_BUTTON */
/** @} */

