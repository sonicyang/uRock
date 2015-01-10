// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

#include "gfx.h"

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

static color_t	colors[256];
static int		multiply;
static coord_t	w, h;
static size_t	ldiff;

static coord_t	lastmousex = 0;
static coord_t	lastmousey = 0;
static boolean	mousemoved = false;


void I_ShutdownGraphics(void)
{
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
}

#if 0
int xlatekey(void)
{

    int rc;

    switch(rc = XKeycodeToKeysym(X_display, X_event.xkey.keycode, 0))
    {
      case XK_Left:	rc = KEY_LEFTARROW;	break;
      case XK_Right:	rc = KEY_RIGHTARROW;	break;
      case XK_Down:	rc = KEY_DOWNARROW;	break;
      case XK_Up:	rc = KEY_UPARROW;	break;
      case XK_Escape:	rc = KEY_ESCAPE;	break;
      case XK_Return:	rc = KEY_ENTER;		break;
      case XK_Tab:	rc = KEY_TAB;		break;
      case XK_F1:	rc = KEY_F1;		break;
      case XK_F2:	rc = KEY_F2;		break;
      case XK_F3:	rc = KEY_F3;		break;
      case XK_F4:	rc = KEY_F4;		break;
      case XK_F5:	rc = KEY_F5;		break;
      case XK_F6:	rc = KEY_F6;		break;
      case XK_F7:	rc = KEY_F7;		break;
      case XK_F8:	rc = KEY_F8;		break;
      case XK_F9:	rc = KEY_F9;		break;
      case XK_F10:	rc = KEY_F10;		break;
      case XK_F11:	rc = KEY_F11;		break;
      case XK_F12:	rc = KEY_F12;		break;

      case XK_BackSpace:
      case XK_Delete:	rc = KEY_BACKSPACE;	break;

      case XK_Pause:	rc = KEY_PAUSE;		break;

      case XK_KP_Equal:
      case XK_equal:	rc = KEY_EQUALS;	break;

      case XK_KP_Subtract:
      case XK_minus:	rc = KEY_MINUS;		break;

      case XK_Shift_L:
      case XK_Shift_R:
	rc = KEY_RSHIFT;
	break;

      case XK_Control_L:
      case XK_Control_R:
	rc = KEY_RCTRL;
	break;

      case XK_Alt_L:
      case XK_Meta_L:
      case XK_Alt_R:
      case XK_Meta_R:
	rc = KEY_RALT;
	break;

      default:
	if (rc >= XK_space && rc <= XK_asciitilde)
	    rc = rc - XK_space + ' ';
	if (rc >= 'A' && rc <= 'Z')
	    rc = rc - 'A' + 'a';
	break;
    }

    return rc;

}
#endif

void I_GetEvent(void)
{
    event_t event;

    #if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
		GEventMouse		mev;
	#endif

	#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
		ginputGetMouseStatus(0, &mev);
		event.type = ev_mouse;
		event.data1 = mev.current_buttons & (GINPUT_MOUSE_BTN_LEFT|GINPUT_MOUSE_BTN_RIGHT|GINPUT_MOUSE_BTN_MIDDLE);
		if ((mev.current_buttons ^ mev.last_buttons) & (GINPUT_MOUSE_BTN_LEFT|GINPUT_MOUSE_BTN_RIGHT|GINPUT_MOUSE_BTN_MIDDLE)) {
			event.data2 = event.data3 = 0;
		} else {
			event.data2 = (mev.x - lastmousex) << 2;
			event.data3 = (lastmousey - mev.y) << 2;
			if (event.data2 || event.data3) {
				lastmousex = mev.x;
				lastmousey = mev.y;
				if (mev.x/multiply != w/2 && mev.y/multiply != h/2)	{
					D_PostEvent(&event);
					mousemoved = false;
				} else
					mousemoved = true;
			}
		}

	#endif
#if 0
    event_t event;

    // put event-grabbing stuff in here
    XNextEvent(X_display, &X_event);
    switch (X_event.type)
    {
      case KeyPress:
	event.type = ev_keydown;
	event.data1 = xlatekey();
	D_PostEvent(&event);
	// fprintf(stderr, "k");
	break;
      case KeyRelease:
	event.type = ev_keyup;
	event.data1 = xlatekey();
	D_PostEvent(&event);
	// fprintf(stderr, "ku");
	break;
      case ButtonPress:
	event.type = ev_mouse;
	event.data1 =
	    (X_event.xbutton.state & Button1Mask)
	    | (X_event.xbutton.state & Button2Mask ? 2 : 0)
	    | (X_event.xbutton.state & Button3Mask ? 4 : 0)
	    | (X_event.xbutton.button == Button1)
	    | (X_event.xbutton.button == Button2 ? 2 : 0)
	    | (X_event.xbutton.button == Button3 ? 4 : 0);
	event.data2 = event.data3 = 0;
	D_PostEvent(&event);
	// fprintf(stderr, "b");
	break;
      case ButtonRelease:
	event.type = ev_mouse;
	event.data1 =
	    (X_event.xbutton.state & Button1Mask)
	    | (X_event.xbutton.state & Button2Mask ? 2 : 0)
	    | (X_event.xbutton.state & Button3Mask ? 4 : 0);
	// suggest parentheses around arithmetic in operand of |
	event.data1 =
	    event.data1
	    ^ (X_event.xbutton.button == Button1 ? 1 : 0)
	    ^ (X_event.xbutton.button == Button2 ? 2 : 0)
	    ^ (X_event.xbutton.button == Button3 ? 4 : 0);
	event.data2 = event.data3 = 0;
	D_PostEvent(&event);
	// fprintf(stderr, "bu");
	break;
      case MotionNotify:
	event.type = ev_mouse;
	event.data1 =
	    (X_event.xmotion.state & Button1Mask)
	    | (X_event.xmotion.state & Button2Mask ? 2 : 0)
	    | (X_event.xmotion.state & Button3Mask ? 4 : 0);
	event.data2 = (X_event.xmotion.x - lastmousex) << 2;
	event.data3 = (lastmousey - X_event.xmotion.y) << 2;

	if (event.data2 || event.data3)
	{
	    lastmousex = X_event.xmotion.x;
	    lastmousey = X_event.xmotion.y;
	    if (X_event.xmotion.x != X_width/2 &&
		X_event.xmotion.y != X_height/2)
	    {
		D_PostEvent(&event);
		// fprintf(stderr, "m");
		mousemoved = false;
	    } else
	    {
		mousemoved = true;
	    }
	}
	break;
	
      case Expose:
      case ConfigureNotify:
	break;
	
      default:
	if (doShm && X_event.type == X_shmeventtype) shmFinished = true;
	break;
    }
#endif
}

//
// I_StartTic
//
void I_StartTic (void)
{
	I_GetEvent();

#if 0
    // Warp the pointer back to the middle of the window
    //  or it will wander off - that is, the game will
    //  loose input focus within X11.
    if (grabMouse)
    {
	if (!--doPointerWarp)
	{
	    XWarpPointer( X_display,
			  None,
			  X_mainWindow,
			  0, 0,
			  0, 0,
			  X_width/2, X_height/2);

	    doPointerWarp = POINTER_WARP_COUNTDOWN;
	}
    }
#endif

    mousemoved = false;
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
	register byte		*p;
	register coord_t	x, y;

	gdispStreamStart(0, 0, w*multiply, h*multiply);

	if (multiply == 1) {
		for(p = screens[0], y=0; y < h; y++, p+=ldiff) {
			for(x = 0; x < w; x++, p++)
				gdispStreamColor(colors[*p]);
		}
	} else {
		for(p = screens[0], y=0; y < h; y++, p+=ldiff) {
			for(x = 0; x < w; x++, p++) {
				gdispStreamColor(colors[*p]);
				gdispStreamColor(colors[*p]);
			}
			for(p -= w, x = 0; x < w; x++, p++) {
				gdispStreamColor(colors[*p]);
				gdispStreamColor(colors[*p]);
			}
		}
	}
	gdispStreamStop();

	// Force a display update if the controller supports it
	gdispFlush();
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
    int	i;

    for (i=0 ; i<256 ; i++, palette+=3) {
    	colors[i] = RGB2COLOR(gammatable[usegamma][palette[0]], gammatable[usegamma][palette[1]], gammatable[usegamma][palette[2]]);
    }
}


void I_InitGraphics(void)
{
	screens[0] = gfxAlloc(SCREENWIDTH*SCREENHEIGHT);

	multiply = 1;
    if (M_CheckParm("-2") || (gdispGetWidth() >= SCREENWIDTH*2 && gdispGetHeight() >= SCREENHEIGHT*2))
    	multiply = 2;

    w = gdispGetWidth()/multiply;
	if (w > SCREENWIDTH)	w = SCREENWIDTH;
	ldiff = SCREENWIDTH - w;

	h = gdispGetHeight()/multiply;
	if (h > SCREENHEIGHT)	h = SCREENHEIGHT;

	#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
		ginputGetMouse(0);
	#endif
}
