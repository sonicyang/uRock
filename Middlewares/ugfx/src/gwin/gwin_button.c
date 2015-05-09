/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/gwin_button.c
 * @brief   GWIN sub-system button code
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_BUTTON

#include "gwin_class.h"

// Parameters for various shapes
#define RND_CNR_SIZE			5		// Rounded corner size for rounded buttons
#define ARROWHEAD_DIVIDER		0		// What fraction of the length for the arrow head. Use 0 for 45 degree arrow head.
#define ARROWBODY_DIVIDER		2		// What fraction of the width for the arrow body
#define TOP_FADE				50		// (TOP_FADE/255)% fade to white for top of button
#define BOTTOM_FADE				25		// (BOTTOM_FADE/255)% fade to black for bottom of button

// Our pressed state
#define GBUTTON_FLG_PRESSED		(GWIN_FIRST_CONTROL_FLAG<<0)

#if GINPUT_NEED_MOUSE
	// A mouse down has occurred over the button
	static void MouseDown(GWidgetObject *gw, coord_t x, coord_t y) {
		(void) x; (void) y;
		gw->g.flags |= GBUTTON_FLG_PRESSED;
		_gwinUpdate((GHandle)gw);
	}

	// A mouse up has occurred (it may or may not be over the button)
	static void MouseUp(GWidgetObject *gw, coord_t x, coord_t y) {
		(void) x; (void) y;
		gw->g.flags &= ~GBUTTON_FLG_PRESSED;
		_gwinUpdate((GHandle)gw);

		#if !GWIN_BUTTON_LAZY_RELEASE
			// If the mouse up was not over the button then cancel the event
			if (x < 0 || y < 0 || x >= gw->g.width || y >= gw->g.height)
				return;
		#endif

		_gwinSendEvent(&gw->g, GEVENT_GWIN_BUTTON);
	}
#endif

#if GINPUT_NEED_TOGGLE
	// A toggle off has occurred
	static void ToggleOff(GWidgetObject *gw, uint16_t role) {
		(void) role;
		gw->g.flags &= ~GBUTTON_FLG_PRESSED;
		_gwinUpdate((GHandle)gw);
	}

	// A toggle on has occurred
	static void ToggleOn(GWidgetObject *gw, uint16_t role) {
		(void) role;
		gw->g.flags |= GBUTTON_FLG_PRESSED;
		_gwinUpdate((GHandle)gw);
		// Trigger the event on button down (different than for mouse/touch)
		_gwinSendEvent(&gw->g, GEVENT_GWIN_BUTTON);
	}

	static void ToggleAssign(GWidgetObject *gw, uint16_t role, uint16_t instance) {
		(void) role;
		((GButtonObject *)gw)->toggle = instance;
	}

	static uint16_t ToggleGet(GWidgetObject *gw, uint16_t role) {
		(void) role;
		return ((GButtonObject *)gw)->toggle;
	}
#endif

// The button VMT table
static const gwidgetVMT buttonVMT = {
	{
		"Button",				// The classname
		sizeof(GButtonObject),	// The object size
		_gwidgetDestroy,		// The destroy routine
		_gwidgetuRedraw,			// The redraw routine
		0,						// The after-clear routine
	},
	gwinButtonDraw_Normal,			// The default drawing routine
	#if GINPUT_NEED_MOUSE
		{
			MouseDown,				// Process mouse down events
			MouseUp,				// Process mouse up events
			0,						// Process mouse move events (NOT USED)
		},
	#endif
	#if GINPUT_NEED_TOGGLE
		{
			1,						// 1 toggle role
			ToggleAssign,			// Assign Toggles
			ToggleGet,				// Get Toggles
			ToggleOff,				// Process toggle off events
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

GHandle gwinGButtonCreate(GDisplay *g, GButtonObject *gw, const GWidgetInit *pInit) {
	if (!(gw = (GButtonObject *)_gwidgetCreate(g, &gw->w, pInit, &buttonVMT)))
		return 0;

	#if GINPUT_NEED_TOGGLE
		gw->toggle = GWIDGET_NO_INSTANCE;
	#endif
	gwinSetVisible((GHandle)gw, pInit->g.show);
	return (GHandle)gw;
}

bool_t gwinButtonIsPressed(GHandle gh) {
	if (gh->vmt != (gwinVMT *)&buttonVMT)
		return FALSE;

	return (gh->flags & GBUTTON_FLG_PRESSED) ? TRUE : FALSE;
}

/*----------------------------------------------------------
 * Custom Draw Routines
 *----------------------------------------------------------*/

static const GColorSet *getDrawColors(GWidgetObject *gw) {
	if (!(gw->g.flags & GWIN_FLG_SYSENABLED))	return &gw->pstyle->disabled;
	if ((gw->g.flags & GBUTTON_FLG_PRESSED))	return &gw->pstyle->pressed;
	return &gw->pstyle->enabled;
}

#if GWIN_FLAT_STYLING
	void gwinButtonDraw_Normal(GWidgetObject *gw, void *param) {
		const GColorSet *	pcol;
		(void)				param;

		if (gw->g.vmt != (gwinVMT *)&buttonVMT)	return;
		pcol = getDrawColors(gw);

		gdispGFillStringBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width-1, gw->g.height-1, gw->text, gw->g.font, pcol->text, pcol->fill, justifyCenter);
		gdispGDrawLine(gw->g.display, gw->g.x+gw->g.width-1, gw->g.y, gw->g.x+gw->g.width-1, gw->g.y+gw->g.height-1, pcol->edge);
		gdispGDrawLine(gw->g.display, gw->g.x, gw->g.y+gw->g.height-1, gw->g.x+gw->g.width-2, gw->g.y+gw->g.height-1, pcol->edge);
	}
#else
	void gwinButtonDraw_Normal(GWidgetObject *gw, void *param) {
		const GColorSet *	pcol;
		fixed				alpha;
		fixed				dalpha;
		coord_t				i;
		color_t				tcol, bcol;
		(void)				param;

		if (gw->g.vmt != (gwinVMT *)&buttonVMT)	return;
		pcol = getDrawColors(gw);
	
		/* Fill the box blended from variants of the fill color */
		tcol = gdispBlendColor(White, pcol->fill, TOP_FADE);
		bcol = gdispBlendColor(Black, pcol->fill, BOTTOM_FADE);
		dalpha = FIXED(255)/gw->g.height;
		for(alpha = 0, i = 0; i < gw->g.height; i++, alpha += dalpha)
			gdispGDrawLine(gw->g.display, gw->g.x, gw->g.y+i, gw->g.x+gw->g.width-2, gw->g.y+i, gdispBlendColor(bcol, tcol, NONFIXED(alpha)));

		gdispGDrawStringBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width-1, gw->g.height-1, gw->text, gw->g.font, pcol->text, justifyCenter);
		gdispGDrawLine(gw->g.display, gw->g.x+gw->g.width-1, gw->g.y, gw->g.x+gw->g.width-1, gw->g.y+gw->g.height-1, pcol->edge);
		gdispGDrawLine(gw->g.display, gw->g.x, gw->g.y+gw->g.height-1, gw->g.x+gw->g.width-2, gw->g.y+gw->g.height-1, pcol->edge);
	}
#endif

#if GDISP_NEED_ARC
	void gwinButtonDraw_Rounded(GWidgetObject *gw, void *param) {
		const GColorSet *	pcol;
		(void)				param;

		if (gw->g.vmt != (gwinVMT *)&buttonVMT)	return;
		pcol = getDrawColors(gw);

		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
		if (gw->g.width >= 2*RND_CNR_SIZE+10) {
			gdispGFillRoundedBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, RND_CNR_SIZE-1, pcol->fill);
			gdispGDrawStringBox(gw->g.display, gw->g.x+1, gw->g.y+RND_CNR_SIZE, gw->g.width-2, gw->g.height-(2*RND_CNR_SIZE), gw->text, gw->g.font, pcol->text, justifyCenter);
			gdispGDrawRoundedBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, RND_CNR_SIZE, pcol->edge);
		} else {
			gdispGFillStringBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, gw->text, gw->g.font, pcol->text, pcol->fill, justifyCenter);
			gdispGDrawBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, pcol->edge);
		}
	}
#endif

#if GDISP_NEED_ELLIPSE
	void gwinButtonDraw_Ellipse(GWidgetObject *gw, void *param) {
		const GColorSet *	pcol;
		(void)				param;

		if (gw->g.vmt != (gwinVMT *)&buttonVMT)	return;
		pcol = getDrawColors(gw);

		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
		gdispGFillEllipse(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width/2-1, gw->g.height/2-1, pcol->fill);
		gdispGDrawStringBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, gw->text, gw->g.font, pcol->text, justifyCenter);
		gdispGDrawEllipse(gw->g.display, gw->g.x, gw->g.y, gw->g.width/2, gw->g.height/2, pcol->edge);
	}
#endif

#if GDISP_NEED_CONVEX_POLYGON
	void gwinButtonDraw_ArrowUp(GWidgetObject *gw, void *param) {
		const GColorSet *	pcol;
		(void)				param;
		point				arw[7];

		if (gw->g.vmt != (gwinVMT *)&buttonVMT)	return;
		pcol = getDrawColors(gw);

		// Create the arrow polygon
		arw[0].x = (gw->g.width-1)/2;				// Point center
		arw[0].y = 0;								// Arrow start
		arw[3].y = gw->g.height-1;					// Arrow end
		#if ARROWHEAD_DIVIDER == 0
			if (gw->g.height <= arw[0].x) {
				arw[1].y = arw[3].y;				// End of head
				arw[1].x = arw[0].x+arw[3].y;		// Width of head  (side 1)
				arw[2].x = arw[1].x;				// Width of shaft (side 1)
				arw[4].x = arw[0].x-arw[3].y;		// Width of head  (side 2)
				arw[6].x = arw[4].x;				// Width of shaft (side 2)
			} else {
				arw[1].y = arw[0].x;
				arw[1].x = arw[0].x << 1;
				arw[2].x = arw[0].x + arw[0].x/ARROWBODY_DIVIDER;
				arw[4].x = arw[0].x - arw[0].x/ARROWBODY_DIVIDER;
				arw[6].x = 0;
			}
		#else
			arw[1].y = gw->g.height/ARROWHEAD_DIVIDER;
			arw[1].x = arw[0].x << 1;
			arw[2].x = arw[0].x + arw[0].x/ARROWBODY_DIVIDER;
			arw[4].x = arw[0].x - arw[0].x/ARROWBODY_DIVIDER;
			arw[6].x = 0;
		#endif

		// Fill in the rest from the special points
		/* arw[0].x set */											/* arw[0].y set */
		/* arw[1].x set */											/* arw[1].y set */
		/* arw[2].x set */											arw[2].y = arw[1].y;
		arw[3].x = arw[2].x;										/* arw[3].y set */
		/* arw[4].x set */											arw[4].y = arw[3].y;
		arw[5].x = arw[4].x;										arw[5].y = arw[1].y;
		/* arw[6].x set */											arw[6].y = arw[1].y;

		// Draw
		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
		gdispGFillConvexPoly(gw->g.display, gw->g.x, gw->g.y, arw, 7, pcol->fill);
		gdispGDrawPoly(gw->g.display, gw->g.x, gw->g.y, arw, 7, pcol->edge);
		gdispGDrawStringBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, gw->text, gw->g.font, pcol->text, justifyCenter);
	}

	void gwinButtonDraw_ArrowDown(GWidgetObject *gw, void *param) {
		const GColorSet *	pcol;
		(void)				param;
		point				arw[7];

		if (gw->g.vmt != (gwinVMT *)&buttonVMT)	return;
		pcol = getDrawColors(gw);

		// Create the arrow polygon
		arw[0].x = (gw->g.width-1)/2;				// Point center
		arw[0].y = gw->g.height-1;					// Arrow start
		arw[3].y = 0;								// Arrow end
		#if ARROWHEAD_DIVIDER == 0
			if (gw->g.height <= arw[0].x) {
				arw[1].y = arw[3].y;				// End of head
				arw[1].x = arw[0].x+arw[0].y;		// Width of head  (side 1)
				arw[2].x = arw[1].x;				// Width of shaft (side 1)
				arw[4].x = arw[0].x-arw[0].y;		// Width of head  (side 2)
				arw[6].x = arw[4].x;				// Width of shaft (side 2)
			} else {
				arw[1].y = arw[0].y - arw[0].x;
				arw[1].x = arw[0].x << 1;
				arw[2].x = arw[0].x + arw[0].x/ARROWBODY_DIVIDER;
				arw[4].x = arw[0].x - arw[0].x/ARROWBODY_DIVIDER;
				arw[6].x = 0;
			}
		#else
			arw[1].y = arw[0].y - gw->g.height/ARROWHEAD_DIVIDER;
			arw[1].x = arw[0].x << 1;
			arw[2].x = arw[0].x + arw[0].x/ARROWBODY_DIVIDER;
			arw[4].x = arw[0].x - arw[0].x/ARROWBODY_DIVIDER;
			arw[6].x = 0;
		#endif

		// Fill in the rest from the special points
		/* arw[0].x set */											/* arw[0].y set */
		/* arw[1].x set */											/* arw[1].y set */
		/* arw[2].x set */											arw[2].y = arw[1].y;
		arw[3].x = arw[2].x;										/* arw[3].y set */
		/* arw[4].x set */											arw[4].y = arw[3].y;
		arw[5].x = arw[4].x;										arw[5].y = arw[1].y;
		/* arw[6].x set */											arw[6].y = arw[1].y;

		// Draw
		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
		gdispGFillConvexPoly(gw->g.display, gw->g.x, gw->g.y, arw, 7, pcol->fill);
		gdispGDrawPoly(gw->g.display, gw->g.x, gw->g.y, arw, 7, pcol->edge);
		gdispGDrawStringBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, gw->text, gw->g.font, pcol->text, justifyCenter);
	}

	void gwinButtonDraw_ArrowLeft(GWidgetObject *gw, void *param) {
		const GColorSet *	pcol;
		(void)				param;
		point				arw[7];

		if (gw->g.vmt != (gwinVMT *)&buttonVMT)	return;
		pcol = getDrawColors(gw);

		// Create the arrow polygon
		arw[0].y = (gw->g.height-1)/2;				// Point center
		arw[0].x = 0;								// Arrow start
		arw[3].x = gw->g.width-1;					// Arrow end
		#if ARROWHEAD_DIVIDER == 0
			if (gw->g.width <= arw[0].y) {
				arw[1].x = arw[3].x;				// End of head
				arw[1].y = arw[0].y+arw[3].x;		// Width of head  (side 1)
				arw[2].y = arw[1].y;				// Width of shaft (side 1)
				arw[4].y = arw[0].y-arw[3].x;		// Width of head  (side 2)
				arw[6].y = arw[4].y;				// Width of shaft (side 2)
			} else {
				arw[1].x = arw[0].y;
				arw[1].y = arw[0].y << 1;
				arw[2].y = arw[0].y + arw[0].y/ARROWBODY_DIVIDER;
				arw[4].y = arw[0].y - arw[0].y/ARROWBODY_DIVIDER;
				arw[6].y = 0;
			}
		#else
			arw[1].x = gw->g.width/ARROWHEAD_DIVIDER;
			arw[1].y = arw[0].y << 1;
			arw[2].y = arw[0].y + arw[0].y/ARROWBODY_DIVIDER;
			arw[4].y = arw[0].y - arw[0].y/ARROWBODY_DIVIDER;
			arw[6].y = 0;
		#endif

		// Fill in the rest from the special points
		/* arw[0].x set */											/* arw[0].y set */
		/* arw[1].x set */											/* arw[1].y set */
		arw[2].x = arw[1].x;										/* arw[2].y set */
		/* arw[3].y set */											arw[3].y = arw[2].y;
		arw[4].x = arw[3].x;										/* arw[4].y set */
		arw[5].x = arw[1].x;										arw[5].y = arw[4].y;
		arw[6].x = arw[1].x;										/* arw[6].y set */

		// Draw
		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
		gdispGFillConvexPoly(gw->g.display, gw->g.x, gw->g.y, arw, 7, pcol->fill);
		gdispGDrawPoly(gw->g.display, gw->g.x, gw->g.y, arw, 7, pcol->edge);
		gdispGDrawStringBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, gw->text, gw->g.font, pcol->text, justifyCenter);
	}

	void gwinButtonDraw_ArrowRight(GWidgetObject *gw, void *param) {
		const GColorSet *	pcol;
		(void)				param;
		point				arw[7];

		if (gw->g.vmt != (gwinVMT *)&buttonVMT)	return;
		pcol = getDrawColors(gw);

		// Create the arrow polygon
		arw[0].y = (gw->g.height-1)/2;				// Point center
		arw[0].x = gw->g.width-1;					// Arrow start
		arw[3].x = 0;								// Arrow end
		#if ARROWHEAD_DIVIDER == 0
			if (gw->g.width <= arw[0].y) {
				arw[1].x = arw[3].x;				// End of head
				arw[1].y = arw[0].y+arw[0].x;		// Width of head  (side 1)
				arw[2].y = arw[1].y;				// Width of shaft (side 1)
				arw[4].y = arw[0].y-arw[0].x;		// Width of head  (side 2)
				arw[6].y = arw[4].y;				// Width of shaft (side 2)
			} else {
				arw[1].x = arw[0].x - arw[0].y;
				arw[1].y = arw[0].y << 1;
				arw[2].y = arw[0].y + arw[0].y/ARROWBODY_DIVIDER;
				arw[4].y = arw[0].y - arw[0].y/ARROWBODY_DIVIDER;
				arw[6].y = 0;
			}
		#else
			arw[1].x = arw[0].x - gw->g.width/ARROWHEAD_DIVIDER;
			arw[1].y = arw[0].y << 1;
			arw[2].y = arw[0].y + arw[0].y/ARROWBODY_DIVIDER;
			arw[4].y = arw[0].y - arw[0].y/ARROWBODY_DIVIDER;
			arw[6].y = 0;
		#endif

		// Fill in the rest from the special points
		/* arw[0].x set */											/* arw[0].y set */
		/* arw[1].x set */											/* arw[1].y set */
		arw[2].x = arw[1].x;										/* arw[2].y set */
		/* arw[3].y set */											arw[3].y = arw[2].y;
		arw[4].x = arw[3].x;										/* arw[4].y set */
		arw[5].x = arw[1].x;										arw[5].y = arw[4].y;
		arw[6].x = arw[1].x;										/* arw[6].y set */

		// Draw
		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
		gdispGFillConvexPoly(gw->g.display, gw->g.x, gw->g.y, arw, 7, pcol->fill);
		gdispGDrawPoly(gw->g.display, gw->g.x, gw->g.y, arw, 7, pcol->edge);
		gdispGDrawStringBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, gw->text, gw->g.font, pcol->text, justifyCenter);
	}
#endif

#if GDISP_NEED_IMAGE || defined(__DOXYGEN__)
	void gwinButtonDraw_Image(GWidgetObject *gw, void *param) {
		const GColorSet *	pcol;
		coord_t				sy;

		if (gw->g.vmt != (gwinVMT *)&buttonVMT)	return;
		pcol = getDrawColors(gw);

		if (!(gw->g.flags & GWIN_FLG_SYSENABLED)) {
			sy = 2 * gw->g.height;
		} else if ((gw->g.flags & GBUTTON_FLG_PRESSED)) {
			sy = gw->g.height;
		} else {
			sy = 0;
		}

		gdispGImageDraw(gw->g.display, (gdispImage *)param, gw->g.x, gw->g.y, gw->g.width, gw->g.height, 0, sy);
		gdispGDrawStringBox(gw->g.display, gw->g.x+1, gw->g.y+1, gw->g.width-2, gw->g.height-2, gw->text, gw->g.font, pcol->text, justifyCenter);
	}
#endif

#endif /* GFX_USE_GWIN && GWIN_NEED_BUTTON */
