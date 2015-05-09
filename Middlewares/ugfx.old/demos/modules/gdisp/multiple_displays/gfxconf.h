/*
 * Copyright (c) 2012, 2013, Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2012, 2013, Andrew Hannam aka inmarket
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _GFXCONF_H
#define _GFXCONF_H

//* The operating system to use. One of these must be defined - preferably in your Makefile */
//#define GFX_USE_OS_CHIBIOS	FALSE
//#define GFX_USE_OS_WIN32		FALSE
//#define GFX_USE_OS_LINUX		FALSE
//#define GFX_USE_OS_OSX		FALSE

/* GFX sub-systems to turn on */
#define GFX_USE_GDISP			TRUE

/* Features for the GDISP sub-system. */
#define GDISP_NEED_VALIDATION	TRUE
#define GDISP_NEED_CLIP			TRUE
#define GDISP_NEED_TEXT			TRUE

#define GDISP_INCLUDE_FONT_UI2	TRUE

#define GDISP_TOTAL_DISPLAYS	2

/* Uncomment the following lines if you want to use multiple displays on
 * different controllers.
 *
 * Change the definitions to suit your hardware.
 * Currently all controllers must use the same pixel format.
 *
 * Remember that GDISP_TOTAL_DISPLAYS above must match the **Total**
 * number of displays in your system across all controllers.
 *
 * Optionally, you can also specify hardware characteristics that are common to
 * all your controllers. This significantly improves code and speed efficiency
 * as the program doesn't have to detect the hardware method to use on each call.
 *
 * Hardware definitions can be set to:
 * 		- TRUE		- all controllers support this routine
 * 		- FALSE		- no controllers support this routine
 * 		- if not specified then the code auto-detects the hardware.
 *
 * e.g
 * 		#define GDISP_HARDWARE_STREAM_WRITE	FALSE
 * 		#define GDISP_HARDWARE_STREAM_READ	FALSE
 * 		#define GDISP_HARDWARE_DRAWPIXEL	TRUE
 * 		#define GDISP_HARDWARE_FILLS		TRUE
 */
//#define GDISP_TOTAL_CONTROLLERS			2
//#define GDISP_CONTROLLER_LIST				GDISPVMT_Win32, GDISPVMT_Win32
//#define GDISP_CONTROLLER_DISPLAYS			1, 1
//#define GDISP_PIXELFORMAT					GDISP_PIXELFORMAT_RGB888

#endif /* _GFXCONF_H */
