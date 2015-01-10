/**
 * This file has a different license to the rest of the GFX system.
 * You can copy, modify and distribute this file as you see fit.
 * You do not need to publish your source modifications to this file.
 * The only thing you are not permitted to do is to relicense it
 * under a different license.
 */

#ifndef _GFXCONF_H
#define _GFXCONF_H

/* The operating system to use. One of these must be defined - preferably in your Makefile */
//#define GFX_USE_OS_CHIBIOS	FALSE
//#define GFX_USE_OS_WIN32		FALSE
//#define GFX_USE_OS_LINUX		FALSE
//#define GFX_USE_OS_OSX		FALSE

/* GFX subsystems to turn on */
#define GFX_USE_GDISP			TRUE
#define GFX_USE_GEVENT			TRUE
#define GFX_USE_GTIMER			TRUE
#define GFX_USE_GINPUT			TRUE
//#define GFX_USE_GAUDIN			FALSE
//#define GFX_USE_GAUDOUT			FALSE

/* Features for the GDISP subsystem */
//#define GDISP_NEED_AUTOFLUSH		FALSE
//#define GDISP_NEED_TIMERFLUSH		FALSE
#define GDISP_NEED_VALIDATION		TRUE
#define GDISP_NEED_CONTROL			TRUE
#define GDISP_NEED_STREAMING		TRUE
#define GDISP_NEED_TEXT				TRUE

#define GDISP_INCLUDE_FONT_UI2		TRUE

/* Features for the GINPUT subsystem. */
#define GINPUT_NEED_MOUSE		TRUE
//#define GINPUT_NEED_KEYBOARD	FALSE
#define GINPUT_NEED_TOGGLE		FALSE
//#define GINPUT_NEED_DIAL		FALSE

#define GDISP_DEFAULT_ORIENTATION		GDISP_ROTATE_LANDSCAPE

#endif /* _GFXCONF_H */
