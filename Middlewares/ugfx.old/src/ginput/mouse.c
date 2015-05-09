/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/mouse.c
 * @brief   GINPUT mouse/touch code.
 *
 * @defgroup Mouse Mouse
 * @ingroup GINPUT
 * @{
 */
#include "gfx.h"

#if (GFX_USE_GINPUT && GINPUT_NEED_MOUSE) || defined(__DOXYGEN__)

#include "src/ginput/driver_mouse.h"

#if GINPUT_MOUSE_NEED_CALIBRATION
	#if !defined(GFX_USE_GDISP) || !GFX_USE_GDISP
		#error "GINPUT: GFX_USE_GDISP must be defined when mouse or touch calibration is required"
	#endif

	#include <string.h>							// Required for memcpy

	#define GINPUT_MOUSE_CALIBRATION_FONT		"* Double"
	#define GINPUT_MOUSE_CALIBRATION_FONT2		"* Narrow"
	#define GINPUT_MOUSE_CALIBRATION_TEXT		"Calibration"
	#define GINPUT_MOUSE_CALIBRATION_ERROR_TEXT	"Failed - Please try again!"
	#define GINPUT_MOUSE_CALIBRATION_SAME_TEXT	"Error: Same Reading - Check Driver!"

	#if GINPUT_MOUSE_MAX_CALIBRATION_ERROR < 0
		#define GINPUT_MOUSE_CALIBRATION_POINTS		3
	#else
		#define GINPUT_MOUSE_CALIBRATION_POINTS		4
	#endif

	typedef struct Calibration_t {
		float	ax;
		float	bx;
		float	cx;
		float	ay;
		float	by;
		float	cy;
	} Calibration;
#endif

typedef struct MousePoint_t {
	coord_t		x, y;
} MousePoint;

static GTIMER_DECL(MouseTimer);

static struct MouseConfig_t {
	MouseReading					t;
	MousePoint						movepos;
	MousePoint						clickpos;
	systemticks_t					clicktime;
	uint16_t						last_buttons;
	uint16_t						flags;
			#define FLG_INIT_DONE		0x8000
			#define FLG_CLICK_TIMER		0x0001
			#define FLG_IN_CAL			0x0010
			#define FLG_CAL_OK			0x0020
			#define FLG_CAL_SAVED		0x0040
			#define FLG_CAL_FREE		0x0080
			#define FLG_CAL_RAW			0x0100
	#if GINPUT_MOUSE_NEED_CALIBRATION
		GMouseCalibrationSaveRoutine	fnsavecal;
		GMouseCalibrationLoadRoutine	fnloadcal;
		Calibration						caldata;
	#endif
	GDisplay *							display;
} MouseConfig;

void _tsOrientClip(MouseReading *pt, GDisplay *g, bool_t doClip) {
	coord_t		w, h;

	w = gdispGGetWidth(g);
	h = gdispGGetHeight(g);

	#if GDISP_NEED_CONTROL && !GINPUT_MOUSE_NO_ROTATION
		switch(gdispGGetOrientation(g)) {
			case GDISP_ROTATE_0:
				break;
			case GDISP_ROTATE_90:
				{
					coord_t t = pt->x;
					pt->x = w - 1 - pt->y;
					pt->y = t;
				}
				break;
			case GDISP_ROTATE_180:
				pt->x = w - 1 - pt->x;
				pt->y = h - 1 - pt->y;
				break;
			case GDISP_ROTATE_270:
				{
					coord_t t = pt->y;
					pt->y = h - 1 - pt->x;
					pt->x = t;
				}
				break;
			default:
				break;
		}
	#endif

	if (doClip) {
		if (pt->x < 0)	pt->x = 0;
		else if (pt->x >= w) pt->x = w-1;
		if (pt->y < 0)	pt->y = 0;
		else if (pt->y >= h) pt->y = h-1;
	}
}

#if GINPUT_MOUSE_NEED_CALIBRATION
	static inline void _tsSetIdentity(Calibration *c) {
		c->ax = 1;
		c->bx = 0;
		c->cx = 0;
		c->ay = 0;
		c->by = 1;
		c->cy = 0;
	}

	static inline void _tsDrawCross(const MousePoint *pp) {
		gdispGDrawLine(MouseConfig.display, pp->x-15, pp->y, pp->x-2, pp->y, White);
		gdispGDrawLine(MouseConfig.display, pp->x+2, pp->y, pp->x+15, pp->y, White);
		gdispGDrawLine(MouseConfig.display, pp->x, pp->y-15, pp->x, pp->y-2, White);
		gdispGDrawLine(MouseConfig.display, pp->x, pp->y+2, pp->x, pp->y+15, White);

		gdispGDrawLine(MouseConfig.display, pp->x-15, pp->y+15, pp->x-7, pp->y+15, RGB2COLOR(184,158,131));
		gdispGDrawLine(MouseConfig.display, pp->x-15, pp->y+7, pp->x-15, pp->y+15, RGB2COLOR(184,158,131));

		gdispGDrawLine(MouseConfig.display, pp->x-15, pp->y-15, pp->x-7, pp->y-15, RGB2COLOR(184,158,131));
		gdispGDrawLine(MouseConfig.display, pp->x-15, pp->y-7, pp->x-15, pp->y-15, RGB2COLOR(184,158,131));

		gdispGDrawLine(MouseConfig.display, pp->x+7, pp->y+15, pp->x+15, pp->y+15, RGB2COLOR(184,158,131));
		gdispGDrawLine(MouseConfig.display, pp->x+15, pp->y+7, pp->x+15, pp->y+15, RGB2COLOR(184,158,131));

		gdispGDrawLine(MouseConfig.display, pp->x+7, pp->y-15, pp->x+15, pp->y-15, RGB2COLOR(184,158,131));
		gdispGDrawLine(MouseConfig.display, pp->x+15, pp->y-15, pp->x+15, pp->y-7, RGB2COLOR(184,158,131));
	}

	static inline void _tsClearCross(const MousePoint *pp) {
		gdispGFillArea(MouseConfig.display, pp->x - 15, pp->y - 15, 42, 42, uBlue);
	}

	static inline void _tsTransform(MouseReading *pt, const Calibration *c) {
		pt->x = (coord_t) (c->ax * pt->x + c->bx * pt->y + c->cx);
		pt->y = (coord_t) (c->ay * pt->x + c->by * pt->y + c->cy);
	}

	static inline void _tsDo3PointCalibration(const MousePoint *cross, const MousePoint *points, GDisplay *g, Calibration *c) {
		float		dx;
		coord_t		c0, c1, c2;

		#if GDISP_NEED_CONTROL
			/* Convert all cross points back to GDISP_ROTATE_0 convention
			 * before calculating the calibration matrix.
			 */
			switch(gdispGGetOrientation(g)) {
			case GDISP_ROTATE_90:
				c0 = cross[0].y;
				c1 = cross[1].y;
				c2 = cross[2].y;
				break;
			case GDISP_ROTATE_180:
				c0 = c1 = c2 = gdispGGetWidth(g) - 1;
				c0 -= cross[0].x;
				c1 -= cross[1].x;
				c2 -= cross[2].x;
				break;
			case GDISP_ROTATE_270:
				c0 = c1 = c2 = gdispGGetHeight(g) - 1;
				c0 -= cross[0].y;
				c1 -= cross[1].y;
				c2 -= cross[2].y;
				break;
			case GDISP_ROTATE_0:
			default:
				c0 = cross[0].x;
				c1 = cross[1].x;
				c2 = cross[2].x;
				break;
			}
		#else
			(void)	g;

			c0 = cross[0].x;
			c1 = cross[1].x;
			c2 = cross[2].x;
		#endif

		/* Compute all the required determinants */
		dx  = (float)(points[0].x - points[2].x) * (float)(points[1].y - points[2].y)
				- (float)(points[1].x - points[2].x) * (float)(points[0].y - points[2].y);

		c->ax = ((float)(c0 - c2) * (float)(points[1].y - points[2].y)
				- (float)(c1 - c2) * (float)(points[0].y - points[2].y)) / dx;
		c->bx = ((float)(c1 - c2) * (float)(points[0].x - points[2].x)
				- (float)(c0 - c2) * (float)(points[1].x - points[2].x)) / dx;
		c->cx = (c0 * ((float)points[1].x * (float)points[2].y - (float)points[2].x * (float)points[1].y)
				- c1 * ((float)points[0].x * (float)points[2].y - (float)points[2].x * (float)points[0].y)
				+ c2 * ((float)points[0].x * (float)points[1].y - (float)points[1].x * (float)points[0].y)) / dx;

		#if GDISP_NEED_CONTROL
			switch(gdispGGetOrientation(g)) {
			case GDISP_ROTATE_90:
				c0 = c1 = c2 = gdispGGetWidth(g) - 1;
				c0 -= cross[0].x;
				c1 -= cross[1].x;
				c2 -= cross[2].x;
				break;
			case GDISP_ROTATE_180:
				c0 = c1 = c2 = gdispGGetHeight(g) - 1;
				c0 -= cross[0].y;
				c1 -= cross[1].y;
				c2 -= cross[2].y;
				break;
			case GDISP_ROTATE_270:
				c0 = cross[0].x;
				c1 = cross[1].x;
				c2 = cross[2].x;
				break;
			case GDISP_ROTATE_0:
			default:
				c0 = cross[0].y;
				c1 = cross[1].y;
				c2 = cross[2].y;
				break;
			}
		#else
			c0 = cross[0].y;
			c1 = cross[1].y;
			c2 = cross[2].y;
		#endif

		c->ay = ((float)(c0 - c2) * (float)(points[1].y - points[2].y)
				- (float)(c1 - c2) * (float)(points[0].y - points[2].y)) / dx;
		c->by = ((float)(c1 - c2) * (float)(points[0].x - points[2].x)
				- (float)(c0 - c2) * (float)(points[1].x - points[2].x)) / dx;
		c->cy = (c0 * ((float)points[1].x * (float)points[2].y - (float)points[2].x * (float)points[1].y)
				- c1 * ((float)points[0].x * (float)points[2].y - (float)points[2].x * (float)points[0].y)
				+ c2 * ((float)points[0].x * (float)points[1].y - (float)points[1].x * (float)points[0].y)) / dx;
	}
#endif

#if GINPUT_MOUSE_READ_CYCLES > 1
	static void get_raw_reading(MouseReading *pt) {
		int32_t x, y, z;
		unsigned i;

		x = y = z = 0;
		for(i = 0; i < GINPUT_MOUSE_READ_CYCLES; i++) {
			ginput_lld_mouse_get_reading(pt);
			x += pt->x;
			y += pt->y;
			z += pt->z;
		}

		/* Take the average of the readings */
		pt->x = x / GINPUT_MOUSE_READ_CYCLES;
		pt->y = y / GINPUT_MOUSE_READ_CYCLES;
		pt->z = z / GINPUT_MOUSE_READ_CYCLES;
	}
#else
	#define get_raw_reading(pt)		ginput_lld_mouse_get_reading(pt)
#endif

static void get_calibrated_reading(MouseReading *pt) {
	get_raw_reading(pt);

	#if GINPUT_MOUSE_NEED_CALIBRATION
		_tsTransform(pt, &MouseConfig.caldata);
	#endif

	_tsOrientClip(pt, MouseConfig.display, !(MouseConfig.flags & FLG_CAL_RAW));
}

static void MousePoll(void *param) {
	(void) param;
	GSourceListener	*psl;
	GEventMouse		*pe;
	unsigned 		meta;
	uint16_t		upbtns, dnbtns;
	uint32_t		cdiff;
	uint32_t		mdiff;

	// Save the last mouse state
	MouseConfig.last_buttons = MouseConfig.t.buttons;

	// Get the new mouse reading
	get_calibrated_reading(&MouseConfig.t);

	// Calculate out new event meta value and handle CLICK and CXTCLICK
	dnbtns = MouseConfig.t.buttons & ~MouseConfig.last_buttons;
	upbtns = ~MouseConfig.t.buttons & MouseConfig.last_buttons;
	meta = GMETA_NONE;

	// As the touch moves up we need to return a point at the old position because some
	//	controllers return garbage with the mouse up
	if ((upbtns & GINPUT_MOUSE_BTN_LEFT)) {
		MouseConfig.t.x = MouseConfig.movepos.x;
		MouseConfig.t.y = MouseConfig.movepos.y;
	}

	// Calculate the position difference from our movement reference (update the reference if out of range)
	mdiff = (MouseConfig.t.x - MouseConfig.movepos.x) * (MouseConfig.t.x - MouseConfig.movepos.x) +
		(MouseConfig.t.y - MouseConfig.movepos.y) * (MouseConfig.t.y - MouseConfig.movepos.y);
	if (mdiff > GINPUT_MOUSE_MAX_MOVE_JITTER * GINPUT_MOUSE_MAX_MOVE_JITTER) {
		MouseConfig.movepos.x = MouseConfig.t.x;
		MouseConfig.movepos.y = MouseConfig.t.y;
	}
	
	// Check if the click has moved outside the click area and if so cancel the click
	if ((MouseConfig.flags & FLG_CLICK_TIMER)) {
		cdiff = (MouseConfig.t.x - MouseConfig.clickpos.x) * (MouseConfig.t.x - MouseConfig.clickpos.x) +
			(MouseConfig.t.y - MouseConfig.clickpos.y) * (MouseConfig.t.y - MouseConfig.clickpos.y);
		if (cdiff > GINPUT_MOUSE_MAX_CLICK_JITTER * GINPUT_MOUSE_MAX_CLICK_JITTER)
			MouseConfig.flags &= ~FLG_CLICK_TIMER;
	}

	// Mouse down
	if ((dnbtns & (GINPUT_MOUSE_BTN_LEFT|GINPUT_MOUSE_BTN_RIGHT))) {
		MouseConfig.clickpos.x = MouseConfig.t.x;
		MouseConfig.clickpos.y = MouseConfig.t.y;
		MouseConfig.clicktime = gfxSystemTicks();
		MouseConfig.flags |= FLG_CLICK_TIMER;
		if ((dnbtns & GINPUT_MOUSE_BTN_LEFT))
			meta |= GMETA_MOUSE_DOWN;
	}

	// Mouse up
	if ((upbtns & (GINPUT_MOUSE_BTN_LEFT|GINPUT_MOUSE_BTN_RIGHT))) {
		if ((upbtns & GINPUT_MOUSE_BTN_LEFT))
			meta |= GMETA_MOUSE_UP;
		if ((MouseConfig.flags & FLG_CLICK_TIMER)) {
			if ((upbtns & GINPUT_MOUSE_BTN_LEFT)
					#if GINPUT_MOUSE_CLICK_TIME != TIME_INFINITE
						&& gfxSystemTicks() - MouseConfig.clicktime < gfxMillisecondsToTicks(GINPUT_MOUSE_CLICK_TIME)
					#endif
					)
				meta |= GMETA_MOUSE_CLICK;
			else
				meta |= GMETA_MOUSE_CXTCLICK;
			MouseConfig.flags &= ~FLG_CLICK_TIMER;
		}
	}

	// Send the event to the listeners that are interested.
	psl = 0;
	while ((psl = geventGetSourceListener((GSourceHandle)(&MouseConfig), psl))) {
		if (!(pe = (GEventMouse *)geventGetEventBuffer(psl))) {
			// This listener is missing - save the meta events that have happened
			psl->srcflags |= meta;
			continue;
		}

		// If we haven't really moved (and there are no meta events) don't bother sending the event
		if (mdiff <= GINPUT_MOUSE_MAX_MOVE_JITTER * GINPUT_MOUSE_MAX_MOVE_JITTER && !psl->srcflags
				&& !meta && MouseConfig.last_buttons == MouseConfig.t.buttons && !(psl->listenflags & GLISTEN_MOUSENOFILTER))
			continue;

		// Send the event if we are listening for it
		if (((MouseConfig.t.buttons & GINPUT_MOUSE_BTN_LEFT) && (psl->listenflags & GLISTEN_MOUSEDOWNMOVES))
				|| (!(MouseConfig.t.buttons & GINPUT_MOUSE_BTN_LEFT) && (psl->listenflags & GLISTEN_MOUSEUPMOVES))
				|| (meta && (psl->listenflags & GLISTEN_MOUSEMETA))) {
			pe->type = GINPUT_MOUSE_EVENT_TYPE;
			pe->instance = 0;
			pe->x = MouseConfig.t.x;
			pe->y = MouseConfig.t.y;
			pe->z = MouseConfig.t.z;
			pe->current_buttons = MouseConfig.t.buttons;
			pe->last_buttons = MouseConfig.last_buttons;
			pe->meta = meta;
			if (psl->srcflags) {
				pe->current_buttons |= GINPUT_MISSED_MOUSE_EVENT;
				pe->meta |= psl->srcflags;
				psl->srcflags = 0;
			}
			pe->display = MouseConfig.display;
			geventSendEvent(psl);
		}
	}
}

GSourceHandle ginputGetMouse(uint16_t instance) {
	#if GINPUT_MOUSE_NEED_CALIBRATION
		Calibration		*pc;
	#endif

	// We only support a single mouse instance currently
	//	Instance 9999 is the same as instance 0 except that it installs
	//	a special "raw" calibration if there isn't one we can load.
	if (instance && instance != 9999)
		return 0;

	// Make sure we have a valid mouse display
	if (!MouseConfig.display)
		MouseConfig.display = GDISP;

	// Do we need to initialise the mouse subsystem?
	if (!(MouseConfig.flags & FLG_INIT_DONE)) {
		ginput_lld_mouse_init();

		#if GINPUT_MOUSE_NEED_CALIBRATION
			#if GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE
				if (!MouseConfig.fnloadcal) {
					MouseConfig.fnloadcal = ginput_lld_mouse_calibration_load;
					MouseConfig.flags &= ~FLG_CAL_FREE;
				}
				if (!MouseConfig.fnsavecal)
					MouseConfig.fnsavecal = ginput_lld_mouse_calibration_save;
			#endif
			if (MouseConfig.fnloadcal && (pc = (Calibration *)MouseConfig.fnloadcal(instance))) {
				memcpy(&MouseConfig.caldata, pc, sizeof(MouseConfig.caldata));
				MouseConfig.flags |= (FLG_CAL_OK|FLG_CAL_SAVED);
				if ((MouseConfig.flags & FLG_CAL_FREE))
					gfxFree((void *)pc);
			} else if (instance == 9999) {
				_tsSetIdentity(&MouseConfig.caldata);
				MouseConfig.flags |= (FLG_CAL_OK|FLG_CAL_SAVED|FLG_CAL_RAW);
			} else
				ginputCalibrateMouse(instance);
		#endif

		// Get the first reading
		MouseConfig.last_buttons = 0;
		get_calibrated_reading(&MouseConfig.t);

		// Mark init as done and start the Poll timer
		MouseConfig.flags |= FLG_INIT_DONE;
		gtimerStart(&MouseTimer, MousePoll, 0, TRUE, GINPUT_MOUSE_POLL_PERIOD);
	}

	// Return our structure as the handle
	return (GSourceHandle)&MouseConfig;
}

void ginputSetMouseDisplay(uint16_t instance, GDisplay *g) {
	if (instance)
		return;

	MouseConfig.display = g ? g : GDISP;
}

GDisplay *ginputGetMouseDisplay(uint16_t instance) {
	if (instance)
		return 0;

	return MouseConfig.display;
}

bool_t ginputGetMouseStatus(uint16_t instance, GEventMouse *pe) {
	// Win32 threads don't seem to recognise priority and/or pre-emption
	// so we add a sleep here to prevent 100% polled applications from locking up.
	gfxSleepMilliseconds(1);

	if (instance || (MouseConfig.flags & (FLG_INIT_DONE|FLG_IN_CAL)) != FLG_INIT_DONE)
		return FALSE;

	pe->type = GINPUT_MOUSE_EVENT_TYPE;
	pe->instance = instance;
	pe->x = MouseConfig.t.x;
	pe->y = MouseConfig.t.y;
	pe->z = MouseConfig.t.z;
	pe->current_buttons = MouseConfig.t.buttons;
	pe->last_buttons = MouseConfig.last_buttons;
	if (pe->current_buttons & ~pe->last_buttons & GINPUT_MOUSE_BTN_LEFT)
		pe->meta = GMETA_MOUSE_DOWN;
	else if (~pe->current_buttons & pe->last_buttons & GINPUT_MOUSE_BTN_LEFT)
		pe->meta = GMETA_MOUSE_UP;
	else
		pe->meta = GMETA_NONE;
	return TRUE;
}

bool_t ginputCalibrateMouse(uint16_t instance) {
	#if !GINPUT_MOUSE_NEED_CALIBRATION
		(void) instance;
		
		return FALSE;
	#else

		const coord_t height  =  gdispGGetHeight(MouseConfig.display);
		const coord_t width  =  gdispGGetWidth(MouseConfig.display);
		#if GINPUT_MOUSE_CALIBRATE_EXTREMES
			const MousePoint cross[]  =  {{0, 0},
									{(width - 1) , 0},
									{(width - 1) , (height - 1)},
									{(width / 2), (height / 2)}}; /* Check point */
		#else
			const MousePoint cross[]  =  {{(width / 4), (height / 4)},
									{(width - (width / 4)) , (height / 4)},
									{(width - (width / 4)) , (height - (height / 4))},
									{(width / 2), (height / 2)}}; /* Check point */
		#endif
		MousePoint points[GINPUT_MOUSE_CALIBRATION_POINTS];
		const MousePoint	*pc;
		MousePoint *pt;
		int32_t px, py;
		unsigned i, j;
		font_t	font1, font2;
		#if GINPUT_MOUSE_MAX_CALIBRATION_ERROR >= 0
			unsigned	err;
		#endif

		if (instance || (MouseConfig.flags & FLG_IN_CAL))
			return FALSE;

		font1 = gdispOpenFont(GINPUT_MOUSE_CALIBRATION_FONT);
		font2 = gdispOpenFont(GINPUT_MOUSE_CALIBRATION_FONT2);

		MouseConfig.flags |= FLG_IN_CAL;
		gtimerStop(&MouseTimer);
		MouseConfig.flags &= ~(FLG_CAL_OK|FLG_CAL_SAVED|FLG_CAL_RAW);

		#if GDISP_NEED_CLIP
			gdispGSetClip(MouseConfig.display, 0, 0, width, height);
		#endif

		#if GINPUT_MOUSE_MAX_CALIBRATION_ERROR >= 0
			while(1) {
		#endif
				gdispGClear(MouseConfig.display, uBlue);

				gdispGFillStringBox(MouseConfig.display, 0, 5, width, 30, GINPUT_MOUSE_CALIBRATION_TEXT, font1,  White, uBlue, justifyCenter);

				for(i = 0, pt = points, pc = cross; i < GINPUT_MOUSE_CALIBRATION_POINTS; i++, pt++, pc++) {
					_tsDrawCross(pc);

					do {

						/* Wait for the mouse to be pressed */
						while(get_raw_reading(&MouseConfig.t), !(MouseConfig.t.buttons & GINPUT_MOUSE_BTN_LEFT))
							gfxSleepMilliseconds(20);

						/* Average all the samples while the mouse is down */
						for(px = py = 0, j = 0;
								gfxSleepMilliseconds(20),			/* Settling time between readings */
								get_raw_reading(&MouseConfig.t),
								(MouseConfig.t.buttons & GINPUT_MOUSE_BTN_LEFT);
								j++) {
							px += MouseConfig.t.x;
							py += MouseConfig.t.y;
						}

					} while(!j);

					pt->x = px / j;
					pt->y = py / j;

					_tsClearCross(pc);

					if (i >= 1 && pt->x == (pt-1)->x && pt->y == (pt-1)->y) {
						gdispGFillStringBox(MouseConfig.display, 0, 35, width, 40, GINPUT_MOUSE_CALIBRATION_SAME_TEXT, font2,  uRed, Yellow, justifyCenter);
						gfxSleepMilliseconds(5000);
						gdispGFillArea(MouseConfig.display, 0, 35, width, 40, uBlue);
					}

				}

				/* Apply 3 point calibration algorithm */
				_tsDo3PointCalibration(cross, points, MouseConfig.display, &MouseConfig.caldata);

				 /* Verification of correctness of calibration (optional) :
				 *  See if the 4th point (Middle of the screen) coincides with the calibrated
				 *  result. If point is within +/- Squareroot(ERROR) pixel margin, then successful calibration
				 *  Else, start from the beginning.
				 */
		#if GINPUT_MOUSE_MAX_CALIBRATION_ERROR >= 0
				/* Transform the co-ordinates */
				MouseConfig.t.x = points[3].x;
				MouseConfig.t.y = points[3].y;
				_tsTransform(&MouseConfig.t, &MouseConfig.caldata);
				_tsOrientClip(&MouseConfig.t, MouseConfig.display, FALSE);

				/* Calculate the delta */
				err = (MouseConfig.t.x - cross[3].x) * (MouseConfig.t.x - cross[3].x) +
					(MouseConfig.t.y - cross[3].y) * (MouseConfig.t.y - cross[3].y);

				if (err <= GINPUT_MOUSE_MAX_CALIBRATION_ERROR * GINPUT_MOUSE_MAX_CALIBRATION_ERROR)
					break;

				gdispGFillStringBox(MouseConfig.display, 0, 35, width, 40, GINPUT_MOUSE_CALIBRATION_ERROR_TEXT, font2, uRed, Yellow, justifyCenter);
				gfxSleepMilliseconds(5000);
			}
		#endif

		// Restart everything
		gdispCloseFont(font1);
		gdispCloseFont(font2);
		MouseConfig.flags |= FLG_CAL_OK;
		MouseConfig.last_buttons = 0;
		get_calibrated_reading(&MouseConfig.t);
		MouseConfig.flags &= ~FLG_IN_CAL;
		if ((MouseConfig.flags & FLG_INIT_DONE))
			gtimerStart(&MouseTimer, MousePoll, 0, TRUE, GINPUT_MOUSE_POLL_PERIOD);
		
		// Save the calibration data (if possible)
		if (MouseConfig.fnsavecal) {
			MouseConfig.fnsavecal(instance, (const uint8_t *)&MouseConfig.caldata, sizeof(MouseConfig.caldata));
			MouseConfig.flags |= FLG_CAL_SAVED;
		}

		// Clear the screen using the GWIN default background color
		#if GFX_USE_GWIN
			gdispGClear(MouseConfig.display, gwinGetDefaultBgColor());
		#else
			gdispGClear(MouseConfig.display, Black);
		#endif
	
		return TRUE;
	#endif
}

/* Set the routines to save and fetch calibration data.
 * This function should be called before first calling ginputGetMouse() for a particular instance
 *	as the ginputGetMouse() routine may attempt to fetch calibration data and perform a startup calibration if there is no way to get it.
 *	If this is called after ginputGetMouse() has been called and the driver requires calibration storage, it will immediately save the data is has already obtained.
 * The 'requireFree' parameter indicates if the fetch buffer must be free()'d to deallocate the buffer provided by the Fetch routine.
 */
void ginputSetMouseCalibrationRoutines(uint16_t instance, GMouseCalibrationSaveRoutine fnsave, GMouseCalibrationLoadRoutine fnload, bool_t requireFree) {
	#if GINPUT_MOUSE_NEED_CALIBRATION
		if (instance)
			return;

		MouseConfig.fnloadcal = fnload;
		MouseConfig.fnsavecal = fnsave;
		if (requireFree)
			MouseConfig.flags |= FLG_CAL_FREE;
		else
			MouseConfig.flags &= ~FLG_CAL_FREE;
		#if GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE
			if (!MouseConfig.fnloadcal) {
				MouseConfig.fnloadcal = ginput_lld_mouse_calibration_load;
				MouseConfig.flags &= ~FLG_CAL_FREE;
			}
			if (!MouseConfig.fnsavecal)
				MouseConfig.fnsavecal = ginput_lld_mouse_calibration_save;
		#endif
		if (MouseConfig.fnsavecal && (MouseConfig.flags & (FLG_CAL_OK|FLG_CAL_SAVED)) == FLG_CAL_OK) {
			MouseConfig.fnsavecal(instance, (const uint8_t *)&MouseConfig.caldata, sizeof(MouseConfig.caldata));
			MouseConfig.flags |= FLG_CAL_SAVED;
		}
	#else
		(void)instance, (void)fnsave, (void)fnload, (void)requireFree;
	#endif
}

/* Test if a particular mouse instance requires routines to save its calibration data. */
bool_t ginputRequireMouseCalibrationStorage(uint16_t instance) {
	if (instance)
		return FALSE;

	#if GINPUT_MOUSE_NEED_CALIBRATION && !GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE
		return TRUE;
	#else
		return FALSE;
	#endif
}

/* Wake up the mouse driver from an interrupt service routine (there may be new readings available) */
void ginputMouseWakeup(void) {
	gtimerJab(&MouseTimer);
}

/* Wake up the mouse driver from an interrupt service routine (there may be new readings available) */
void ginputMouseWakeupI(void) {
	gtimerJabI(&MouseTimer);
}

#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */
/** @} */
