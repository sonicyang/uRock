/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/checkbox.c
 * @brief   GWIN sub-system button code.
 *
 * @defgroup Checkbox Checkbox
 * @ingroup GWIN
 *
 * @{
 */

#include "gfx.h"

#if (GFX_USE_GWIN && GWIN_NEED_CHECKBOX) || defined(__DOXYGEN__)

#include "src/gwin/class_gwin.h"

// Our checked state
#define GCHECKBOX_FLG_CHECKED		(GWIN_FIRST_CONTROL_FLAG<<0)

// Send the checkbox event
static void SendCheckboxEvent(GWidgetObject *gw) {
	GSourceListener	*	psl;
	GEvent *			pe;
	#define pce			((GEventGWinCheckbox *)pe)

	// Trigger a GWIN Checkbox Event
	psl = 0;
	while ((psl = geventGetSourceListener(GWIDGET_SOURCE, psl))) {
		if (!(pe = geventGetEventBuffer(psl)))
			continue;
		pce->type = GEVENT_GWIN_CHECKBOX;
		pce->checkbox = &gw->g;
		pce->isChecked = (gw->g.flags & GCHECKBOX_FLG_CHECKED) ? TRUE : FALSE;
		geventSendEvent(psl);
	}

	#undef pce
}

#if GINPUT_NEED_MOUSE
	static void MouseDown(GWidgetObject *gw, coord_t x, coord_t y) {
		(void) x; (void) y;
		gw->g.flags ^= GCHECKBOX_FLG_CHECKED;
		_gwidgetRedraw((GHandle)gw);
		SendCheckboxEvent(gw);
	}
#endif

#if GINPUT_NEED_TOGGLE
	static void ToggleOn(GWidgetObject *gw, uint16_t role) {
		(void) role;
		gw->g.flags ^= GCHECKBOX_FLG_CHECKED;
		_gwidgetRedraw((GHandle)gw);
		SendCheckboxEvent(gw);
	}

	static void ToggleAssign(GWidgetObject *gw, uint16_t role, uint16_t instance) {
		(void) role;
		((GCheckboxObject *)gw)->toggle = instance;
	}

	static uint16_t ToggleGet(GWidgetObject *gw, uint16_t role) {
		(void) role;
		return ((GCheckboxObject *)gw)->toggle;
	}
#endif

// The checkbox VMT table
static const gwidgetVMT checkboxVMT = {
	{
		"Checkbox",				// The classname
		sizeof(GCheckboxObject),// The object size
		_gwidgetDestroy,		// The destroy routine
		_gwidgetRedraw,			// The redraw routine
		0,						// The after-clear routine
	},
	gwinCheckboxDraw_CheckOnLeft,	// The default drawing routine
	#if GINPUT_NEED_MOUSE
		{
			MouseDown,				// Process mouse down events
			0,						// Process mouse up events (NOT USED)
			0,						// Process mouse move events (NOT USED)
		},
	#endif
	#if GINPUT_NEED_TOGGLE
		{
			1,						// 1 toggle role
			ToggleAssign,			// Assign Toggles
			ToggleGet,				// Get Toggles
			0,						// Process toggle off events (NOT USED)
			ToggleOn,				// Process toggle on events
		},
	#endif
	#if GINPUT_NEED_DIAL
		{
			0,						// No dial roles
			0,						// Assign Dials (NOT USED)
			0,						// Get Dials (NOT USED)
			0,						// Process dial move events (NOT USED)
		},
	#endif
};

GHandle gwinGCheckboxCreate(GDisplay *g, GCheckboxObject *gb, const GWidgetInit *pInit) {
	if (!(gb = (GCheckboxObject *)_gwidgetCreate(g, &gb->w, pInit, &checkboxVMT)))
		return 0;

	#if GINPUT_NEED_TOGGLE
		gb->toggle = GWIDGET_NO_INSTANCE;
	#endif
	gwinSetVisible((GHandle)gb, pInit->g.show);
	return (GHandle)gb;
}

void gwinCheckboxCheck(GHandle gh, bool_t isChecked) {
	if (gh->vmt != (gwinVMT *)&checkboxVMT)
		return;

	if (isChecked) {
		if ((gh->flags & GCHECKBOX_FLG_CHECKED)) return;
		gh->flags |= GCHECKBOX_FLG_CHECKED;
	} else {
		if (!(gh->flags & GCHECKBOX_FLG_CHECKED)) return;
		gh->flags &= ~GCHECKBOX_FLG_CHECKED;
	}
	_gwidgetRedraw(gh);
	SendCheckboxEvent((GWidgetObject *)gh);
}

bool_t gwinCheckboxIsChecked(GHandle gh) {
	if (gh->vmt != (gwinVMT *)&checkboxVMT)
		return FALSE;

	return (gh->flags & GCHECKBOX_FLG_CHECKED) ? TRUE : FALSE;
}

/*----------------------------------------------------------
 * Custom Draw Routines
 *----------------------------------------------------------*/

static const GColorSet *getDrawColors(GWidgetObject *gw) {
	if (!(gw->g.flags & GWIN_FLG_ENABLED))		return &gw->pstyle->disabled;
	if ((gw->g.flags & GCHECKBOX_FLG_CHECKED))	return &gw->pstyle->pressed;
	return &gw->pstyle->enabled;
}

void gwinCheckboxDraw_CheckOnLeft(GWidgetObject *gw, void *param) {
	#define gcw			((GCheckboxObject *)gw)
	coord_t				ld, df;
	const GColorSet *	pcol;
	(void)				param;

	if (gw->g.vmt != (gwinVMT *)&checkboxVMT) return;
	pcol = getDrawColors(gw);

	ld = gw->g.width < gw->g.height ? gw->g.width : gw->g.height;
	gdispGFillArea(gw->g.display, gw->g.x+1, gw->g.y+1, ld, ld-2, gw->pstyle->background);
	gdispGDrawBox(gw->g.display, gw->g.x, gw->g.y, ld, ld, pcol->edge);

	df = ld < 4 ? 1 : 2;
	if (gw->g.flags & GCHECKBOX_FLG_CHECKED)
		gdispGFillArea(gw->g.display, gw->g.x+df, gw->g.y+df, ld-2*df, ld-2*df, pcol->fill);

	gdispGFillStringBox(gw->g.display, gw->g.x+ld+1, gw->g.y, gw->g.width-ld-1, gw->g.height, gw->text, gw->g.font, pcol->text, gw->pstyle->background, justifyLeft);
	#undef gcw
}

void gwinCheckboxDraw_CheckOnRight(GWidgetObject *gw, void *param) {
	#define gcw			((GCheckboxObject *)gw)
	coord_t				ep, ld, df;
	const GColorSet *	pcol;
	(void)				param;

	if (gw->g.vmt != (gwinVMT *)&checkboxVMT) return;
	pcol = getDrawColors(gw);

	ld = gw->g.width < gw->g.height ? gw->g.width : gw->g.height;
	ep = gw->g.width-ld-1;
	gdispGFillArea(gw->g.display, gw->g.x+ep-1, gw->g.y+1, ld, ld-2, gw->pstyle->background);
	gdispGDrawBox(gw->g.display, gw->g.x+ep, gw->g.y, ld, ld, pcol->edge);

	df = ld < 4 ? 1 : 2;
	if (gw->g.flags & GCHECKBOX_FLG_CHECKED)
		gdispGFillArea(gw->g.display, gw->g.x+ep+df, gw->g.y+df, ld-2*df, ld-2*df, pcol->fill);

	gdispGFillStringBox(gw->g.display, gw->g.x, gw->g.y, ep-1, gw->g.height, gw->text, gw->g.font, pcol->text, gw->pstyle->background, justifyRight);
	#undef gcw
}

#endif /* (GFX_USE_GWIN && GWIN_NEED_CHECKBOX) */
/** @} */

