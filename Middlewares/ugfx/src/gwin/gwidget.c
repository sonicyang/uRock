/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_WIDGET

#include <string.h>

#include "src/gwin/class_gwin.h"

/* Our listener for events for widgets */
static GListener			gl;

/* Our default style - a white background theme */
const GWidgetStyle WhiteWidgetStyle = {
	HTML2COLOR(0xFFFFFF),			// window background

	// enabled color set
	{
		HTML2COLOR(0x000000),		// text
		HTML2COLOR(0x404040),		// edge
		HTML2COLOR(0xE0E0E0),		// fill
		HTML2COLOR(0xE0E0E0),		// progress - inactive area
	},

	// disabled color set
	{
		HTML2COLOR(0xC0C0C0),		// text
		HTML2COLOR(0x808080),		// edge
		HTML2COLOR(0xE0E0E0),		// fill
		HTML2COLOR(0xC0E0C0),		// progress - active area
	},

	// pressed color set
	{
		HTML2COLOR(0x404040),		// text
		HTML2COLOR(0x404040),		// edge
		HTML2COLOR(0x808080),		// fill
		HTML2COLOR(0x00E000),		// progress - active area
	},
};

/* Our black style */
const GWidgetStyle BlackWidgetStyle = {
	HTML2COLOR(0x000000),			// window background

	// enabled color set
	{
		HTML2COLOR(0xC0C0C0),		// text
		HTML2COLOR(0xC0C0C0),		// edge
		HTML2COLOR(0x606060),		// fill
		HTML2COLOR(0x404040),		// progress - inactive area
	},

	// disabled color set
	{
		HTML2COLOR(0x808080),		// text
		HTML2COLOR(0x404040),		// edge
		HTML2COLOR(0x404040),		// fill
		HTML2COLOR(0x004000),		// progress - active area
	},

	// pressed color set
	{
		HTML2COLOR(0xFFFFFF),		// text
		HTML2COLOR(0xC0C0C0),		// edge
		HTML2COLOR(0xE0E0E0),		// fill
		HTML2COLOR(0x008000),		// progress - active area
	},
};

static const GWidgetStyle *	defaultStyle = &BlackWidgetStyle;

/* We use these everywhere in this file */
#define gw		((GWidgetObject *)gh)
#define wvmt	((gwidgetVMT *)gh->vmt)

/* Process an event */
static void gwidgetEvent(void *param, GEvent *pe) {
	#define gh		QItem2GWindow(qi)
	#define pme		((GEventMouse *)pe)
	#define pte		((GEventToggle *)pe)
	#define pde		((GEventDial *)pe)

	const gfxQueueASyncItem *	qi;
	#if GFX_USE_GINPUT && (GINPUT_NEED_TOGGLE || GINPUT_NEED_DIAL)
		uint16_t				role;
	#endif
	(void)						param;

	// Process various events
	switch (pe->type) {

	#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
	case GEVENT_MOUSE:
	case GEVENT_TOUCH:
		// Cycle through all windows
		for(qi = gfxQueueASyncPeek(&_GWINList); qi; qi = gfxQueueASyncNext(qi)) {

			// check if the widget matches this display
			if (gh->display != pme->display)
				continue;

			// check if it a widget that is enabled and visible
			if ((gh->flags & (GWIN_FLG_WIDGET|GWIN_FLG_ENABLED|GWIN_FLG_VISIBLE)) != (GWIN_FLG_WIDGET|GWIN_FLG_ENABLED|GWIN_FLG_VISIBLE))
				continue;

			// Are we captured?
			if ((gw->g.flags & GWIN_FLG_MOUSECAPTURE)) {
				if ((pme->last_buttons & ~pme->current_buttons & GINPUT_MOUSE_BTN_LEFT)) {
					gw->g.flags &= ~GWIN_FLG_MOUSECAPTURE;
					if (wvmt->MouseUp)
						wvmt->MouseUp(gw, pme->x - gw->g.x, pme->y - gw->g.y);
				} else if (wvmt->MouseMove)
					wvmt->MouseMove(gw, pme->x - gw->g.x, pme->y - gw->g.y);

			// We are not captured - look for mouse downs over the widget
			} else if ((~pme->last_buttons & pme->current_buttons & GINPUT_MOUSE_BTN_LEFT)
					&& pme->x >= gw->g.x && pme->x < gw->g.x + gw->g.width
					&& pme->y >= gw->g.y && pme->y < gw->g.y + gw->g.height) {
				gw->g.flags |= GWIN_FLG_MOUSECAPTURE;
				if (wvmt->MouseDown)
					wvmt->MouseDown(gw, pme->x - gw->g.x, pme->y - gw->g.y);
			}
		}
		break;
	#endif

	#if GFX_USE_GINPUT && GINPUT_NEED_TOGGLE
	case GEVENT_TOGGLE:
		// Cycle through all windows
		for(qi = gfxQueueASyncPeek(&_GWINList); qi; qi = gfxQueueASyncNext(qi)) {

			// check if it a widget that is enabled and visible
			if ((gh->flags & (GWIN_FLG_WIDGET|GWIN_FLG_ENABLED|GWIN_FLG_VISIBLE)) != (GWIN_FLG_WIDGET|GWIN_FLG_ENABLED|GWIN_FLG_VISIBLE))
				continue;

			for(role = 0; role < wvmt->toggleroles; role++) {
				if (wvmt->ToggleGet(gw, role) == pte->instance) {
					if (pte->on) {
						if (wvmt->ToggleOn)
							wvmt->ToggleOn(gw, role);
					} else {
						if (wvmt->ToggleOff)
							wvmt->ToggleOff(gw, role);
					}
				}
			}
		}
		break;
	#endif

	#if GFX_USE_GINPUT && GINPUT_NEED_DIAL
	case GEVENT_DIAL:
		// Cycle through all windows
		for(qi = gfxQueueASyncPeek(&_GWINList); qi; qi = gfxQueueASyncNext(qi)) {

			// check if it a widget that is enabled and visible
			if ((gh->flags & (GWIN_FLG_WIDGET|GWIN_FLG_ENABLED|GWIN_FLG_VISIBLE)) != (GWIN_FLG_WIDGET|GWIN_FLG_ENABLED|GWIN_FLG_VISIBLE))
				continue;

			for(role = 0; role < wvmt->dialroles; role++) {
				if (wvmt->DialGet(gw, role) == pte->instance) {
					if (wvmt->DialMove)
						wvmt->DialMove(gw, role, pde->value, pde->maxvalue);
				}
			}
		}
		break;
	#endif

	default:
		break;
	}

	#undef gh
	#undef pme
	#undef pte
	#undef pde
}

#if GFX_USE_GINPUT && GINPUT_NEED_TOGGLE
	static GHandle FindToggleUser(uint16_t instance) {
		#define gh					QItem2GWindow(qi)
		const gfxQueueASyncItem *	qi;
		uint16_t					role;

		for(qi = gfxQueueASyncPeek(&_GWINList); qi; qi = gfxQueueASyncNext(qi)) {
			if (!(gh->flags & GWIN_FLG_WIDGET))		// check if it a widget
				continue;

			for(role = 0; role < wvmt->toggleroles; role++) {
				if (wvmt->ToggleGet(gw, role) == instance)
					return gh;
			}
		}
		return 0;
		#undef gh
	}
#endif

#if GFX_USE_GINPUT && GINPUT_NEED_DIAL
	static GHandle FindDialUser(uint16_t instance) {
		#define gh					QItem2GWindow(qi)
		const gfxQueueASyncItem *	qi;
		uint16_t					role;

		for(qi = gfxQueueASyncPeek(&_GWINList); qi; qi = gfxQueueASyncNext(qi)) {
			if (!(gh->flags & GWIN_FLG_WIDGET))		// check if it a widget
				continue;

			for(role = 0; role < wvmt->dialroles; role++) {
				if (wvmt->DialGet(gw, role) == instance)
					return gh;
			}
		}
		return 0;
		#undef gh
	}
#endif

void _gwidgetInit(void)
{
	geventListenerInit(&gl);
	geventRegisterCallback(&gl, gwidgetEvent, 0);
}

void _gwidgetDeinit(void)
{
	/* ToDo */
}

GHandle _gwidgetCreate(GDisplay *g, GWidgetObject *pgw, const GWidgetInit *pInit, const gwidgetVMT *vmt) {
	if (!(pgw = (GWidgetObject *)_gwindowCreate(g, &pgw->g, &pInit->g, &vmt->g, GWIN_FLG_WIDGET|GWIN_FLG_ENABLED)))
		return 0;

	pgw->text = pInit->text ? pInit->text : "";
	pgw->fnDraw = pInit->customDraw ? pInit->customDraw : vmt->DefaultDraw;
	pgw->fnParam = pInit->customParam;
	pgw->pstyle = pInit->customStyle ? pInit->customStyle : defaultStyle;

	return 	&pgw->g;
}

void _gwidgetDestroy(GHandle gh) {
	#if GFX_USE_GINPUT && (GINPUT_NEED_TOGGLE || GINPUT_NEED_DIAL)
		uint16_t	role, instance;
	#endif

	// Deallocate the text (if necessary)
	if ((gh->flags & GWIN_FLG_ALLOCTXT)) {
		gh->flags &= ~GWIN_FLG_ALLOCTXT;
		gfxFree((void *)gw->text);
	}

	#if GFX_USE_GINPUT && GINPUT_NEED_TOGGLE
		// Detach any toggles from this object
		for(role = 0; role < wvmt->toggleroles; role++) {
			instance = wvmt->ToggleGet(gw, role);
			if (instance != GWIDGET_NO_INSTANCE) {
				wvmt->ToggleAssign(gw, role, GWIDGET_NO_INSTANCE);
				if (!FindToggleUser(instance))
					geventDetachSource(&gl, ginputGetToggle(instance));
			}
		}
	#endif

	#if GFX_USE_GINPUT && GINPUT_NEED_DIAL
		// Detach any dials from this object
		for(role = 0; role < wvmt->dialroles; role++) {
			instance = wvmt->DialGet(gw, role);
			if (instance != GWIDGET_NO_INSTANCE) {
				wvmt->DialAssign(gw, role, GWIDGET_NO_INSTANCE);
				if (!FindDialUser(instance))
					geventDetachSource(&gl, ginputGetDial(instance));
			}
		}
	#endif

	// Remove any listeners on this object.
	geventDetachSourceListeners((GSourceHandle)gh);
}

void _gwidgetRedraw(GHandle gh) {
	if (!(gh->flags & GWIN_FLG_VISIBLE))
		return;

	#if GDISP_NEED_CLIP
		gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
	#endif

	gw->fnDraw(gw, gw->fnParam);
}

void gwinWidgetClearInit(GWidgetInit *pwi) {
	char		*p;
	unsigned	len;

	for(p = (char *)pwi, len = sizeof(GWidgetInit); len; len--)
		*p++ = 0;
}

void gwinSetDefaultStyle(const GWidgetStyle *pstyle, bool_t updateAll) {
	if (!pstyle)
		pstyle = &BlackWidgetStyle;

	if (updateAll) {
		const gfxQueueASyncItem *	qi;
		GHandle						gh;

		for(qi = gfxQueueASyncPeek(&_GWINList); qi; qi = gfxQueueASyncNext(qi)) {
			gh = QItem2GWindow(qi);
			if ((gh->flags & GWIN_FLG_WIDGET) && ((GWidgetObject *)gh)->pstyle == defaultStyle)
				gwinSetStyle(gh, pstyle);
		}
	}
	gwinSetDefaultBgColor(pstyle->background);
	defaultStyle = pstyle;
}

/**
 * @brief   Get the current default style.
 *
 * @api
 */
const GWidgetStyle *gwinGetDefaultStyle(void) {
	return defaultStyle;
}


void gwinSetText(GHandle gh, const char *text, bool_t useAlloc) {
	if (!(gh->flags & GWIN_FLG_WIDGET))
		return;

	// Dispose of the old string
	if ((gh->flags & GWIN_FLG_ALLOCTXT)) {
		gh->flags &= ~GWIN_FLG_ALLOCTXT;
		if (gw->text) {
			gfxFree((void *)gw->text);
			gw->text = "";
		}
	}

	// Alloc the new text if required
	if (!text || !*text)
		gw->text = "";
	else if (useAlloc) {
		char *str;

		if ((str = gfxAlloc(strlen(text)+1))) {
			gh->flags |= GWIN_FLG_ALLOCTXT;
			strcpy(str, text);
		}
		gw->text = (const char *)str;
	} else
		gw->text = text;
	_gwidgetRedraw(gh);
}

const char *gwinGetText(GHandle gh) {
	if (!(gh->flags & GWIN_FLG_WIDGET))
		return 0;

	return gw->text;
}

void gwinSetStyle(GHandle gh, const GWidgetStyle *pstyle) {
	if (!(gh->flags & GWIN_FLG_WIDGET))
		return;
	gw->pstyle = pstyle ? pstyle : defaultStyle;
	gh->bgcolor = pstyle->background;
	gh->color = pstyle->enabled.text;
	_gwidgetRedraw(gh);
}

const GWidgetStyle *gwinGetStyle(GHandle gh) {
	if (!(gh->flags & GWIN_FLG_WIDGET))
		return 0;

	return gw->pstyle;
}

void gwinSetCustomDraw(GHandle gh, CustomWidgetDrawFunction fn, void *param) {
	if (!(gh->flags & GWIN_FLG_WIDGET))
		return;

	gw->fnDraw = fn ? fn : wvmt->DefaultDraw;
	gw->fnParam = param;
	_gwidgetRedraw(gh);
}

bool_t gwinAttachListener(GListener *pl) {
	return geventAttachSource(pl, GWIDGET_SOURCE, 0);
}

#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
	bool_t gwinAttachMouse(uint16_t instance) {
		GSourceHandle	gsh;

		if (!(gsh = ginputGetMouse(instance)))
			return FALSE;

		return geventAttachSource(&gl, gsh, GLISTEN_MOUSEMETA|GLISTEN_MOUSEDOWNMOVES);
	}
#endif

#if GFX_USE_GINPUT && GINPUT_NEED_TOGGLE
	bool_t gwinAttachToggle(GHandle gh, uint16_t role, uint16_t instance) {
		GSourceHandle	gsh;
		uint16_t		oi;

		// Is this a widget
		if (!(gh->flags & GWIN_FLG_WIDGET))
			return FALSE;

		// Is the role valid
		if (role >= wvmt->toggleroles)
			return FALSE;

		// Is this a valid device
		if (!(gsh = ginputGetToggle(instance)))
			return FALSE;

		// Is this already done?
		oi = wvmt->ToggleGet(gw, role);
		if (instance == oi)
			return TRUE;

		// Remove the old instance
		if (oi != GWIDGET_NO_INSTANCE) {
			wvmt->ToggleAssign(gw, role, GWIDGET_NO_INSTANCE);
			if (!FindToggleUser(oi))
				geventDetachSource(&gl, ginputGetToggle(oi));
		}

		// Assign the new
		wvmt->ToggleAssign(gw, role, instance);
		return geventAttachSource(&gl, gsh, GLISTEN_TOGGLE_ON|GLISTEN_TOGGLE_OFF);
	}
#endif

#if GFX_USE_GINPUT && GINPUT_NEED_DIAL
	bool_t gwinAttachDial(GHandle gh, uint16_t role, uint16_t instance) {
		GSourceHandle	gsh;
		uint16_t		oi;

		if (!(gh->flags & GWIN_FLG_WIDGET))
			return FALSE;

		// Is the role valid
		if (role >= wvmt->dialroles)
			return FALSE;

		// Is this a valid device
		if (!(gsh = ginputGetDial(instance)))
			return FALSE;

		// Is this already done?
		oi = wvmt->DialGet(gw, role);
		if (instance == oi)
			return TRUE;

		// Remove the old instance
		if (oi != GWIDGET_NO_INSTANCE) {
			wvmt->DialAssign(gw, role, GWIDGET_NO_INSTANCE);
			if (!FindDialUser(oi))
				geventDetachSource(&gl, ginputGetDial(oi));
		}

		// Assign the new
		wvmt->DialAssign(gw, role, instance);
		return geventAttachSource(&gl, gsh, 0);
	}
#endif

#endif /* GFX_USE_GWIN && GWIN_NEED_WIDGET */
/** @} */
