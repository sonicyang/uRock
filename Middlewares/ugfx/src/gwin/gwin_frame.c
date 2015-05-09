/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/gwin_frame.c
 * @brief   GWIN sub-system frame code.
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_FRAME

#include "gwin_class.h"

/* Some position values */
#define BUTTON_X		18					// Button Width
#define BUTTON_Y		18					// Button Height
#define BUTTON_I		3					// Button inner margin
#define BUTTON_T		2					// Gap from top of window to button
#define BUTTON_B		2					// Gap from button to the bottom of the frame title area
#define BORDER_L		2					// Left Border
#define BORDER_R		2					// Right Border
#define BORDER_T		(BUTTON_Y+BUTTON_T+BUTTON_B)	// Top Border (Title area)
#define BORDER_B		2					// Bottom Border

/* Internal state flags */
#define GWIN_FRAME_USER_FLAGS		(GWIN_FRAME_CLOSE_BTN|GWIN_FRAME_MINMAX_BTN)
#define GWIN_FRAME_CLOSE_PRESSED	(GWIN_FRAME_MINMAX_BTN << 1)
#define GWIN_FRAME_MIN_PRESSED		(GWIN_FRAME_MINMAX_BTN << 2)
#define GWIN_FRAME_MAX_PRESSED		(GWIN_FRAME_MINMAX_BTN << 3)
#define GWIN_FRAME_REDRAW_FRAME		(GWIN_FRAME_MINMAX_BTN << 4)		// Only redraw the frame
#if GWIN_FRAME_CLOSE_BTN < GWIN_FIRST_CONTROL_FLAG
	#error "GWIN Frame: - Flag definitions don't match"
#endif
#if GWIN_FRAME_REDRAW_FRAME > GWIN_LAST_CONTROL_FLAG
	#error "GWIN Frame: - Flag definitions don't match"
#endif

static coord_t BorderSizeL(GHandle gh)	{ (void)gh; return BORDER_L; }
static coord_t BorderSizeR(GHandle gh)	{ (void)gh; return BORDER_R; }
static coord_t BorderSizeT(GHandle gh)	{ (void)gh; return BORDER_T; }
static coord_t BorderSizeB(GHandle gh)	{ (void)gh; return BORDER_B; }

static void forceFrameuRedraw(GWidgetObject *gw) {
	// Force a redraw of just the frame.
	// This is a big naughty but who really cares.
	gw->g.flags |= GWIN_FRAME_REDRAW_FRAME;
	gw->fnDraw(gw, gw->fnParam);
	gw->g.flags &= ~GWIN_FRAME_REDRAW_FRAME;
}

#if GINPUT_NEED_MOUSE
	static void mouseDown(GWidgetObject *gw, coord_t x, coord_t y) {
		coord_t		pos;

		// We must be clicking on the frame button area to be of interest
		if (y < BUTTON_T || y >= BUTTON_T+BUTTON_Y)
			return;

		pos = gw->g.width - (BORDER_R+BUTTON_X);
		if ((gw->g.flags & GWIN_FRAME_CLOSE_BTN)) {
			if (x >= pos && x < pos+BUTTON_X) {
				// Close is pressed - force redraw the frame only
				gw->g.flags |= GWIN_FRAME_CLOSE_PRESSED;
				forceFrameuRedraw(gw);
				return;
			}
			pos -= BUTTON_X;
		}
		if ((gw->g.flags & GWIN_FRAME_MINMAX_BTN)) {
			if (x >= pos && x < pos+BUTTON_X) {
				// Close is pressed - force redraw the frame only
				gw->g.flags |= GWIN_FRAME_MAX_PRESSED;
				forceFrameuRedraw(gw);
				return;
			}
			pos -= BUTTON_X;
			if (x >= pos && x < pos+BUTTON_X) {
				// Close is pressed - force redraw the frame only
				gw->g.flags |= GWIN_FRAME_MIN_PRESSED;
				forceFrameuRedraw(gw);
				return;
			}
			pos -= BUTTON_X;
		}
	}

	static void mouseUp(GWidgetObject *gw, coord_t x, coord_t y) {
		#if GWIN_BUTTON_LAZY_RELEASE
			if ((gw->g.flags & GWIN_FRAME_CLOSE_PRESSED)) {
				// Close is released - destroy the window
				gw->g.flags &= ~(GWIN_FRAME_CLOSE_PRESSED|GWIN_FRAME_MAX_PRESSED|GWIN_FRAME_MIN_PRESSED);
				forceFrameuRedraw(gw);
				_gwinSendEvent(&gw->g, GEVENT_GWIN_CLOSE);
				_gwinDestroy(&gw->g, REDRAW_INSESSION);
				return;
			}
			if ((gw->g.flags & GWIN_FRAME_MAX_PRESSED)) {
				// Max is released - maximize the window
				gw->g.flags &= ~(GWIN_FRAME_CLOSE_PRESSED|GWIN_FRAME_MAX_PRESSED|GWIN_FRAME_MIN_PRESSED);
				forceFrameuRedraw(gw);
				gwinSetMinMax(&gw->g, GWIN_MAXIMIZE);
				return;
			}
			if ((gw->g.flags & GWIN_FRAME_MIN_PRESSED)) {
				// Min is released - minimize the window
				gw->g.flags &= ~(GWIN_FRAME_CLOSE_PRESSED|GWIN_FRAME_MAX_PRESSED|GWIN_FRAME_MIN_PRESSED);
				forceFrameuRedraw(gw);
				gwinSetMinMax(&gw->g, GWIN_MINIMIZE);
				return;
			}
		#else
			// If nothing is pressed we have nothing to do.
			if (!(gw->g.flags & (GWIN_FRAME_CLOSE_PRESSED|GWIN_FRAME_MAX_PRESSED|GWIN_FRAME_MIN_PRESSED)))
				return;

			// We must be releasing over the button
			if (y >= BUTTON_T && y < BUTTON_T+BUTTON_Y) {
				coord_t		pos;

				pos = gw->g.width - (BORDER_R+BUTTON_X);
				if ((gw->g.flags & GWIN_FRAME_CLOSE_BTN)) {
					if ((gw->g.flags & GWIN_FRAME_CLOSE_PRESSED) && x >= pos && x <= pos+BUTTON_X) {
						// Close is released - destroy the window. This is tricky as we already have the drawing lock.
						gw->g.flags &= ~(GWIN_FRAME_CLOSE_PRESSED|GWIN_FRAME_MAX_PRESSED|GWIN_FRAME_MIN_PRESSED);
						forceFrameuRedraw(gw);
						_gwinSendEvent(&gw->g, GEVENT_GWIN_CLOSE);
						_gwinDestroy(&gw->g, REDRAW_INSESSION);
						return;
					}
					pos -= BUTTON_X;
				}
				if ((gw->g.flags & GWIN_FRAME_MINMAX_BTN)) {
					if ((gw->g.flags & GWIN_FRAME_MAX_PRESSED) && x >= pos && x <= pos+BUTTON_X) {
						// Max is released - maximize the window
						gw->g.flags &= ~(GWIN_FRAME_CLOSE_PRESSED|GWIN_FRAME_MAX_PRESSED|GWIN_FRAME_MIN_PRESSED);
						forceFrameuRedraw(gw);
						gwinSetMinMax(&gw->g, GWIN_MAXIMIZE);
						return;
					}
					pos -= BUTTON_X;
					if ((gw->g.flags & GWIN_FRAME_MIN_PRESSED) && x >= pos && x <= pos+BUTTON_X) {
						// Min is released - minimize the window
						gw->g.flags &= ~(GWIN_FRAME_CLOSE_PRESSED|GWIN_FRAME_MAX_PRESSED|GWIN_FRAME_MIN_PRESSED);
						forceFrameuRedraw(gw);
						gwinSetMinMax(&gw->g, GWIN_MINIMIZE);
						return;
					}
					pos -= BUTTON_X;
				}
			}

			// Clear any flags and redraw the frame
			gw->g.flags &= ~(GWIN_FRAME_CLOSE_PRESSED|GWIN_FRAME_MAX_PRESSED|GWIN_FRAME_MIN_PRESSED);
			forceFrameuRedraw(gw);
		#endif
	}
#endif

static const gcontainerVMT frameVMT = {
	{
		{
			"Frame",					// The classname
			sizeof(GFrameObject),		// The object size
			_gcontainerDestroy,			// The destroy routine
			_gcontaineruRedraw,			// The redraw routine
			0,							// The after-clear routine
		},
		gwinFrameDraw_Std,				// The default drawing routine
		#if GINPUT_NEED_MOUSE
			{
				mouseDown,				// Process mouse down event
				mouseUp,				// Process mouse up events
				0,						// Process mouse move events
			},
		#endif
		#if GINPUT_NEED_TOGGLE
			{
				0,						// 1 toggle role
				0,						// Assign Toggles
				0,						// Get Toggles
				0,						// Process toggle off events
				0,						// Process toggle on events
			},
		#endif
		#if GINPUT_NEED_DIAL
			{
				0,						// 1 dial roles
				0,						// Assign Dials
				0,						// Get Dials
				0,						// Process dial move events
			},
		#endif
	},
	BorderSizeL,						// The size of the left border (mandatory)
	BorderSizeT,						// The size of the top border (mandatory)
	BorderSizeR,						// The size of the right border (mandatory)
	BorderSizeB,						// The size of the bottom border (mandatory)
	0,									// A child has been added (optional)
	0,									// A child has been deleted (optional)
};

GHandle gwinGFrameCreate(GDisplay *g, GFrameObject *fo, GWidgetInit *pInit, uint32_t flags) {
	if (!(fo = (GFrameObject *)_gcontainerCreate(g, (GContainerObject *)fo, pInit, &frameVMT)))
		return 0;

	// Make sure we only have "safe" flags.
	flags &= GWIN_FRAME_USER_FLAGS;

	/* Apply flags. We apply these here so the controls above are outside the child area */
	fo->g.flags |= flags;

	gwinSetVisible(&fo->g, pInit->g.show);

	return &fo->g;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Default render routines                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////

void gwinFrameDraw_Transparent(GWidgetObject *gw, void *param) {
	const GColorSet		*pcol;
	coord_t				pos;
	color_t				contrast;
	color_t				btn;
	(void)param;

	if (gw->g.vmt != (gwinVMT *)&frameVMT)
		return;

	pcol = 	(gw->g.flags & GWIN_FLG_SYSENABLED) ? &gw->pstyle->enabled : &gw->pstyle->disabled;
	contrast = gdispContrastColor(pcol->edge);
	btn = gdispBlendColor(pcol->edge, contrast, 128);

	// Render the frame
	gdispGFillStringBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, BORDER_T, gw->text, gw->g.font, contrast, pcol->edge, justifyCenter);
	gdispGFillArea(gw->g.display, gw->g.x, gw->g.y+BORDER_T, BORDER_L, gw->g.height-(BORDER_T+BORDER_B), pcol->edge);
	gdispGFillArea(gw->g.display, gw->g.x+gw->g.width-BORDER_R, gw->g.y+BORDER_T, BORDER_R, gw->g.height-(BORDER_T+BORDER_B), pcol->edge);
	gdispGFillArea(gw->g.display, gw->g.x, gw->g.y+gw->g.height-BORDER_B, gw->g.width, BORDER_B, pcol->edge);

	// Add the buttons
	pos = gw->g.x+gw->g.width - (BORDER_R+BUTTON_X);

	if ((gw->g.flags & GWIN_FRAME_CLOSE_BTN)) {
		if ((gw->g.flags & GWIN_FRAME_CLOSE_PRESSED))
			gdispFillArea(pos, gw->g.y+BUTTON_T, BUTTON_X, BUTTON_Y, btn);
		gdispDrawLine(pos+BUTTON_I, gw->g.y+(BUTTON_T+BUTTON_I), pos+(BUTTON_X-BUTTON_I-1), gw->g.y+(BUTTON_T+BUTTON_Y-BUTTON_I-1), contrast);
		gdispDrawLine(pos+(BUTTON_X-BUTTON_I-1), gw->g.y+(BUTTON_T+BUTTON_I), pos+BUTTON_I, gw->g.y+(BUTTON_T+BUTTON_Y-BUTTON_I-1), contrast);
		pos -= BUTTON_X;
	}

	if ((gw->g.flags & GWIN_FRAME_MINMAX_BTN)) {
		if ((gw->g.flags & GWIN_FRAME_MAX_PRESSED))
			gdispFillArea(pos, gw->g.y+BUTTON_T, BUTTON_X, BUTTON_Y, btn);
		// the symbol
		gdispDrawBox(pos+BUTTON_I, gw->g.y+(BUTTON_T+BUTTON_I), BUTTON_X-2*BUTTON_I, BUTTON_Y-2*BUTTON_I, contrast);
		gdispDrawLine(pos+(BUTTON_I+1), gw->g.y+(BUTTON_T+BUTTON_I+1), pos+(BUTTON_X-BUTTON_I-2), gw->g.y+(BUTTON_T+BUTTON_I+1), contrast);
		gdispDrawLine(pos+(BUTTON_I+1), gw->g.y+(BUTTON_T+BUTTON_I+2), pos+(BUTTON_X-BUTTON_I-2), gw->g.y+(BUTTON_T+BUTTON_I+2), contrast);
		pos -= BUTTON_X;
		if ((gw->g.flags & GWIN_FRAME_MIN_PRESSED))
			gdispFillArea(pos, gw->g.y+BUTTON_T, BUTTON_X, BUTTON_Y, btn);
		gdispDrawLine(pos+BUTTON_I, gw->g.y+(BUTTON_T+BUTTON_Y-BUTTON_I-1), pos+(BUTTON_X-BUTTON_I-1), gw->g.y+(BUTTON_T+BUTTON_Y-BUTTON_I-1), contrast);
		pos -= BUTTON_X;
	}

	// Don't touch the client area
}

void gwinFrameDraw_Std(GWidgetObject *gw, void *param) {
	(void)param;

	if (gw->g.vmt != (gwinVMT *)&frameVMT)
		return;

	// Draw the frame
	gwinFrameDraw_Transparent(gw, param);

	// Drop out if that is all we want to draw
	if ((gw->g.flags & GWIN_FRAME_REDRAW_FRAME))
		return;

	// Draw the client area
	gdispGFillArea(gw->g.display, gw->g.x + BORDER_L, gw->g.y + BORDER_T, gw->g.width - (BORDER_L+BORDER_R), gw->g.height - (BORDER_T+BORDER_B), gw->pstyle->background);
}

#if GDISP_NEED_IMAGE
	void gwinFrameDraw_Image(GWidgetObject *gw, void *param) {
		#define gi			((gdispImage *)param)
		coord_t				x, y, iw, ih, mx, my;

		if (gw->g.vmt != (gwinVMT *)&frameVMT)
			return;

		// Draw the frame
		gwinFrameDraw_Transparent(gw, param);

		// Drop out if that is all we want to draw
		if ((gw->g.flags & GWIN_FRAME_REDRAW_FRAME))
			return;

		// Draw the client area by tiling the image
		mx = gw->g.x+gw->g.width - BORDER_R;
		my = gw->g.y+gw->g.height - BORDER_B;
		for(y = gw->g.y+BORDER_T, ih = gi->height; y < my; y += ih) {
			if (ih > my - y)
				ih = my - y;
			for(x = gw->g.x+BORDER_L, iw = gi->width; x < mx; x += iw) {
				if (iw > mx - x)
					iw = mx - x;
				gdispGImageDraw(gw->g.display, gi, x, y, iw, ih, 0, 0);
			}
		}

		#undef gi
	}
#endif

#endif  /* (GFX_USE_GWIN && GWIN_NEED_FRAME) || defined(__DOXYGEN__) */
