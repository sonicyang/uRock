/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_DRIVER_VMT			GDISPVMT_X11
#include "drivers/multiple/X/gdisp_lld_config.h"
#include "src/gdisp/driver.h"

#ifndef GDISP_FORCE_24BIT	
	#define GDISP_FORCE_24BIT	FALSE
#endif

#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT		480
#endif
#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH		640
#endif

#define GDISP_FLG_READY				(GDISP_FLG_DRIVER<<0)

#if GINPUT_NEED_MOUSE
	/* Include mouse support code */
	#include "src/ginput/driver_mouse.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

static bool_t			initdone;
static Display			*dis;
static int				scr;
static XEvent			evt;
static Colormap			cmap;
static XVisualInfo		vis;
static XContext			cxt;
#if GINPUT_NEED_MOUSE
	static coord_t		mousex, mousey;
	static uint16_t		mousebuttons;
#endif

typedef struct xPriv {
	Pixmap			pix;
	GC 				gc;
	Window			win;
} xPriv;

static void ProcessEvent(GDisplay *g, xPriv *priv) {
	switch(evt.type) {
	case MapNotify:
		XSelectInput(dis, evt.xmap.window, StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
		g->flags |= GDISP_FLG_READY;
		break;
	case UnmapNotify:
		XCloseDisplay(dis);
		exit(0);
		break;
	case Expose:
		XCopyArea(dis, priv->pix, evt.xexpose.window, priv->gc,
			evt.xexpose.x, evt.xexpose.y,
			evt.xexpose.width, evt.xexpose.height,   
			evt.xexpose.x, evt.xexpose.y);
		break;
#if GINPUT_NEED_MOUSE
	case ButtonPress:
		mousex = evt.xbutton.x;
		mousey = evt.xbutton.y;
		switch(evt.xbutton.button){
		case 1:	mousebuttons |= GINPUT_MOUSE_BTN_LEFT;		break;
		case 2:	mousebuttons |= GINPUT_MOUSE_BTN_MIDDLE;	break;
		case 3:	mousebuttons |= GINPUT_MOUSE_BTN_RIGHT;		break;
		case 4:	mousebuttons |= GINPUT_MOUSE_BTN_4;			break;
		}
		#if GINPUT_MOUSE_POLL_PERIOD == TIME_INFINITE
			ginputMouseWakeup();
		#endif
		break;
	case ButtonRelease:
		mousex = evt.xbutton.x;
		mousey = evt.xbutton.y;
		switch(evt.xbutton.button){
		case 1:	mousebuttons &= ~GINPUT_MOUSE_BTN_LEFT;		break;
		case 2:	mousebuttons &= ~GINPUT_MOUSE_BTN_MIDDLE;	break;
		case 3:	mousebuttons &= ~GINPUT_MOUSE_BTN_RIGHT;	break;
		case 4:	mousebuttons &= ~GINPUT_MOUSE_BTN_4;		break;
		}
		#if GINPUT_MOUSE_POLL_PERIOD == TIME_INFINITE
			ginputMouseWakeup();
		#endif
		break;
	case MotionNotify:
		mousex = evt.xmotion.x;
		mousey = evt.xmotion.y;
		#if GINPUT_MOUSE_POLL_PERIOD == TIME_INFINITE
			ginputMouseWakeup();
		#endif
		break;
#endif
	}
}

/* this is the X11 thread which keeps track of all events */
static DECLARE_THREAD_STACK(waXThread, 1024);
static DECLARE_THREAD_FUNCTION(ThreadX, arg) {
	GDisplay	*g;
	(void)arg;

	while(1) {
		gfxSleepMilliseconds(100);
		while(XPending(dis)) {
			XNextEvent(dis, &evt);
			XFindContext(evt.xany.display, evt.xany.window, cxt, (XPointer*)&g);
			ProcessEvent(g, (xPriv *)g->priv);
		}
	}
	return 0;
}
 
static int FatalXIOError(Display *d) {
	(void) d;

	/* The window has closed */
	fprintf(stderr, "GFX Window closed!\n");
	exit(0);
}

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
	XSizeHints				*pSH;
	XSetWindowAttributes	xa;
	XTextProperty			WindowTitle;
	char *					WindowTitleText;
	xPriv					*priv;

	if (!initdone) {
		gfxThreadHandle			hth;

		initdone = TRUE;
		#if GFX_USE_OS_LINUX || GFX_USE_OS_OSX
			XInitThreads();
		#endif

		dis = XOpenDisplay(0);
		scr = DefaultScreen(dis);
		cxt = XUniqueContext();
		XSetIOErrorHandler(FatalXIOError);

		#if GDISP_FORCE_24BIT
			if (!XMatchVisualInfo(dis, scr, 24, TrueColor, &vis)) {
				fprintf(stderr, "Your display has no TrueColor mode\n");
				XCloseDisplay(dis);
				return FALSE;
			}
			cmap = XCreateColormap(dis, RootWindow(dis, scr),
					vis.visual, AllocNone);
		#else
			vis.visual = CopyFromParent;
			vis.depth = DefaultDepth(dis, scr);
			cmap = DefaultColormap(dis, scr);
		#endif
		fprintf(stderr, "Running GFX Window in %d bit color\n", vis.depth);

		if (!(hth = gfxThreadCreate(waXThread, sizeof(waXThread), HIGH_PRIORITY, ThreadX, 0))) {
			fprintf(stderr, "Cannot start X Thread\n");
			XCloseDisplay(dis);
			exit(0);
		}
		#if GFX_USE_OS_LINUX || GFX_USE_OS_OSX
			pthread_detach(hth);
		#endif
		gfxThreadClose(hth);
	}

	g->priv = gfxAlloc(sizeof(xPriv));
	priv = (xPriv *)g->priv;
	g->board = 0;					// No board interface for this driver

	xa.colormap = cmap;
	xa.border_pixel = 0xFFFFFF;
	xa.background_pixel = 0x000000;
	
	priv->win = XCreateWindow(dis, RootWindow(dis, scr), 16, 16,
			GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT,
			0, vis.depth, InputOutput, vis.visual,
			CWBackPixel|CWColormap|CWBorderPixel, &xa);
	XSync(dis, TRUE);
	
	XSaveContext(dis, priv->win, cxt, (XPointer)g);

	{
		char					buf[132];
		sprintf(buf, "uGFX - %u", g->systemdisplay+1);
		WindowTitleText = buf;
		XStringListToTextProperty(&WindowTitleText, 1, &WindowTitle);
		XSetWMName(dis, priv->win, &WindowTitle);
		XSetWMIconName(dis, priv->win, &WindowTitle);
		XSync(dis, TRUE);
	}
			
	pSH = XAllocSizeHints();
	pSH->flags = PSize | PMinSize | PMaxSize;
	pSH->min_width = pSH->max_width = pSH->base_width = GDISP_SCREEN_WIDTH;
	pSH->min_height = pSH->max_height = pSH->base_height = GDISP_SCREEN_HEIGHT;
	XSetWMNormalHints(dis, priv->win, pSH);
	XFree(pSH);
	XSync(dis, TRUE);
	
	priv->pix = XCreatePixmap(dis, priv->win,
				GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT, vis.depth);
	XSync(dis, TRUE);

	priv->gc = XCreateGC(dis, priv->win, 0, 0);
	XSetBackground(dis, priv->gc, BlackPixel(dis, scr));
	XSync(dis, TRUE);

	XSelectInput(dis, priv->win, StructureNotifyMask);
	XMapWindow(dis, priv->win);

	// Wait for the window creation to complete (for safety)
	while(!(((volatile GDisplay *)g)->flags & GDISP_FLG_READY))
		gfxSleepMilliseconds(100);

	/* Initialise the GDISP structure to match */
    g->g.Orientation = GDISP_ROTATE_0;
    g->g.Powermode = powerOn;
    g->g.Backlight = 100;
    g->g.Contrast = 50;
    g->g.Width = GDISP_SCREEN_WIDTH;
    g->g.Height = GDISP_SCREEN_HEIGHT;
    return TRUE;
}

LLDSPEC void gdisp_lld_draw_pixel(GDisplay *g)
{
	xPriv *	priv = (xPriv *)g->priv;
	XColor	col;

	col.red = RED_OF(g->p.color) << 8;
	col.green = GREEN_OF(g->p.color) << 8;
	col.blue = BLUE_OF(g->p.color) << 8;
	XAllocColor(dis, cmap, &col);
	XSetForeground(dis, priv->gc, col.pixel);
	XDrawPoint(dis, priv->pix, priv->gc, (int)g->p.x, (int)g->p.y );
	XDrawPoint(dis, priv->win, priv->gc, (int)g->p.x, (int)g->p.y );
	XFlush(dis);
}

#if GDISP_HARDWARE_FILLS
	LLDSPEC void gdisp_lld_fill_area(GDisplay *g) {
		xPriv *	priv = (xPriv *)g->priv;
		XColor	col;

		col.red = RED_OF(g->p.color) << 8;
		col.green = GREEN_OF(g->p.color) << 8;
		col.blue = BLUE_OF(g->p.color) << 8;
		XAllocColor(dis, cmap, &col);
		XSetForeground(dis, priv->gc, col.pixel);
		XFillRectangle(dis, priv->pix, priv->gc, g->p.x, g->p.y, g->p.cx, g->p.cy);
		XFillRectangle(dis, priv->win, priv->gc, g->p.x, g->p.y, g->p.cx, g->p.cy);
		XFlush(dis);
	}
#endif

#if 0 && GDISP_HARDWARE_BITFILLS
	LLDSPEC void gdisp_lld_blit_area(GDisplay *g) {
		// Start of Bitblit code

		//XImage			bitmap;
		//pixel_t			*bits;
		//	bits = malloc(vis.depth * GDISP_SCREEN_WIDTH * GDISP_SCREEN_HEIGHT);
		//	bitmap = XCreateImage(dis, vis, vis.depth, ZPixmap,
		//				0, bits, GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT,
		//				0, 0);
	}
#endif

#if GDISP_HARDWARE_PIXELREAD
	LLDSPEC	color_t gdisp_lld_get_pixel_color(GDisplay *g) {
		xPriv *	priv = (xPriv *)g->priv;
		XColor	color;
		XImage *img;

		img = XGetImage (dis, priv->pix, g->p.x, g->p.y, 1, 1, AllPlanes, XYPixmap);
		color.pixel = XGetPixel (img, 0, 0);
		XFree(img);
		XQueryColor(dis, cmap, &color);
		return RGB2COLOR(color.red>>8, color.green>>8, color.blue>>8);
	}
#endif

#if GDISP_NEED_SCROLL && GDISP_HARDWARE_SCROLL
	LLDSPEC void gdisp_lld_vertical_scroll(GDisplay *g) {
		xPriv *	priv = (xPriv *)g->priv;

		if (g->p.y1 > 0) {
			XCopyArea(dis, priv->pix, priv->pix, priv->gc, g->p.x, g->p.y+g->p.y1, g->p.cx, g->p.cy-g->p.y1, g->p.x, g->p.y);
			XCopyArea(dis, priv->pix, priv->win, priv->gc, g->p.x, g->p.y, g->p.cx, g->p.cy-g->p.y1, g->p.x, g->p.y);
		} else {
			XCopyArea(dis, priv->pix, priv->pix, priv->gc, g->p.x, g->p.y, g->p.cx, g->p.cy+g->p.y1, g->p.x, g->p.y-g->p.y1);
			XCopyArea(dis, priv->pix, priv->win, priv->gc, g->p.x, g->p.y-g->p.y1, g->p.cx, g->p.cy+g->p.y1, g->p.x, g->p.y-g->p.y1);
		}
	}
#endif

#if GINPUT_NEED_MOUSE

	void ginput_lld_mouse_init(void) {}

	void ginput_lld_mouse_get_reading(MouseReading *pt) {
		pt->x = mousex;
		pt->y = mousey;
		pt->z = (mousebuttons & GINPUT_MOUSE_BTN_LEFT) ? 100 : 0;
		pt->buttons = mousebuttons;
	}

#endif /* GINPUT_NEED_MOUSE */

#endif /* GFX_USE_GDISP */
