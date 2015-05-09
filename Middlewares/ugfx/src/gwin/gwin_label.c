/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file	src/gwin/gwin_label.c
 * @brief	GWIN label widget header file
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_LABEL

#include "gwin_class.h"

// macros to assist in data type conversions
#define gh2obj					((GLabelObject *)gh)
#define gw2obj					((GLabelObject *)gw)

// flags for the GLabelObject
#define GLABEL_FLG_WAUTO		(GWIN_FIRST_CONTROL_FLAG << 0)
#define GLABEL_FLG_HAUTO		(GWIN_FIRST_CONTROL_FLAG << 1)
#define GLABEL_FLG_BORDER		(GWIN_FIRST_CONTROL_FLAG << 2)

// simple: single line with no wrapping
static coord_t getwidth(const char *text, font_t font, coord_t maxwidth) {
	(void) maxwidth;

	return gdispGetStringWidth(text, font)+2;		// Allow one pixel of padding on each side
}

// simple: single line with no wrapping
static coord_t getheight(const char *text, font_t font, coord_t maxwidth) {
	(void) text;
	(void) maxwidth;

	return gdispGetFontMetric(font, fontHeight);
}

static void gwinLabelDefaultDraw(GWidgetObject *gw, void *param);

static const gwidgetVMT labelVMT = {
	{
		"Label",				// The class name
		sizeof(GLabelObject),	// The object size
		_gwidgetDestroy,		// The destroy routine
		_gwidgetuRedraw, 		// The redraw routine
		0,						// The after-clear routine
	},
	gwinLabelDefaultDraw,		// default drawing routine
	#if GINPUT_NEED_MOUSE
		{
			0,						// Process mose down events (NOT USED)
			0,						// Process mouse up events (NOT USED)
			0,						// Process mouse move events (NOT USED)
		},
	#endif
	#if GINPUT_NEED_TOGGLE
		{
			0,						// No toggle role
			0,						// Assign Toggles (NOT USED)
			0,						// Get Toggles (NOT USED)
			0,						// Process toggle off event (NOT USED)
			0,						// Process toggle on event (NOT USED)
		},
	#endif
	#if GINPUT_NEED_DIAL
		{
			0,						// No dial roles
			0,						// Assign Dials (NOT USED)
			0, 						// Get Dials (NOT USED)
			0,						// Procees dial move events (NOT USED)
		},
	#endif
};

GHandle gwinGLabelCreate(GDisplay *g, GLabelObject *widget, GWidgetInit *pInit) {
	uint16_t flags = 0;

	// auto assign width
	if (pInit->g.width <= 0) {

		flags |= GLABEL_FLG_WAUTO;
		pInit->g.width = getwidth(pInit->text, gwinGetDefaultFont(), gdispGGetWidth(g) - pInit->g.x);
	}
 
	// auto assign height
	if (pInit->g.height <= 0) {
		flags |= GLABEL_FLG_HAUTO;
		pInit->g.height = getheight(pInit->text, gwinGetDefaultFont(), gdispGGetWidth(g) - pInit->g.x);
	}

	if (!(widget = (GLabelObject *)_gwidgetCreate(g, &widget->w, pInit, &labelVMT)))
		return 0;

	#if GWIN_LABEL_ATTRIBUTE
		widget->tab = 0;
		widget->attr = 0;
	#endif

	widget->w.g.flags |= flags;	
	gwinSetVisible(&widget->w.g, pInit->g.show);

	return (GHandle)widget;
}

void gwinLabelSetBorder(GHandle gh, bool_t border) {
	// is it a valid handle?
	if (gh->vmt != (gwinVMT *)&labelVMT)
		return;
	
	if (border)
		gh2obj->w.g.flags |= GLABEL_FLG_BORDER;
	else
		gh2obj->w.g.flags &=~ GLABEL_FLG_BORDER;
}

#if GWIN_LABEL_ATTRIBUTE
	void gwinLabelSetAttribute(GHandle gh, coord_t tab, const char* attr) {
		// is it a valid handle?
		if (gh->vmt != (gwinVMT *)&labelVMT)
			return;
	
		gh2obj->tab = tab;
		gh2obj->attr = attr;

		gwinuRedraw(gh); 
	}
#endif // GWIN_LABEL_ATTRIBUTE

static void gwinLabelDefaultDraw(GWidgetObject *gw, void *param) {
	coord_t				w, h;
	color_t				c;
	(void)				param;

	// is it a valid handle?
	if (gw->g.vmt != (gwinVMT *)&labelVMT)
		return;

	w = (gw->g.flags & GLABEL_FLG_WAUTO) ? getwidth(gw->text, gw->g.font, gdispGGetWidth(gw->g.display) - gw->g.x) : gw->g.width;
	h = (gw->g.flags & GLABEL_FLG_HAUTO) ? getheight(gw->text, gw->g.font, gdispGGetWidth(gw->g.display) - gw->g.x) : gw->g.height;
	c = (gw->g.flags & GWIN_FLG_SYSENABLED) ? gw->pstyle->enabled.text : gw->pstyle->disabled.text;

	if (gw->g.width != w || gw->g.height != h) {
		/* Only allow the widget to be resize if it will grow larger.
		 * Growing smaller is problematic because it requires a temporary change in visibility.
		 * This is a work-around for a crashing bug caused by calling gwinResize() in the draw function
		 * (dubious) as it may try to reclaim the drawing lock.
		 */
		if (gw->g.width < w || gw->g.height < h) {
			gwinResize(&gw->g, (w > gw->g.width ? w : gw->g.width), (h > gw->g.height ? h : gw->g.height));
			return;
		}
		w = gw->g.width;
		h = gw->g.height;
	}

	#if GWIN_LABEL_ATTRIBUTE
		if (gw2obj->attr) {
			gdispGFillStringBox(gw->g.display, gw->g.x, gw->g.y, gw2obj->tab, h, gw2obj->attr, gw->g.font, c, gw->pstyle->background, justifyLeft);
			gdispGFillStringBox(gw->g.display, gw->g.x + gw2obj->tab, gw->g.y, w-gw2obj->tab, h, gw->text, gw->g.font, c, gw->pstyle->background, justifyLeft);
		} else
			gdispGFillStringBox(gw->g.display, gw->g.x, gw->g.y, w, h, gw->text, gw->g.font, c, gw->pstyle->background, justifyLeft);
	#else
		gdispGFillStringBox(gw->g.display, gw->g.x, gw->g.y, w, h, gw->text, gw->g.font, c, gw->pstyle->background, justifyLeft);
	#endif

	// render the border (if any)
	if (gw->g.flags & GLABEL_FLG_BORDER)
		gdispGDrawBox(gw->g.display, gw->g.x, gw->g.y, w, h, (gw->g.flags & GWIN_FLG_SYSENABLED) ? gw->pstyle->enabled.edge : gw->pstyle->disabled.edge);
}

#endif // GFX_USE_GWIN && GFX_NEED_LABEL
