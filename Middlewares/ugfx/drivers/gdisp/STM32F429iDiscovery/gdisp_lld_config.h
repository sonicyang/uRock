/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_CONFIG_H
#define _GDISP_LLD_CONFIG_H

#if GFX_USE_GDISP

/*===========================================================================*/
/* Driver hardware support.                                                  */
/*===========================================================================*/

#define GDISP_HARDWARE_DRAWPIXEL		TRUE
#define GDISP_HARDWARE_PIXELREAD		TRUE
#define GDISP_HARDWARE_CONTROL			TRUE
#define GDISP_HARDWARE_STREAM_WRITE          FALSE
#define GDISP_HARDWARE_STREAM_READ           FALSE
#define GDISP_HARDWARE_STREAM_POS            FALSE
#define GDISP_HARDWARE_CLEARS                FALSE
#define GDISP_HARDWARE_FILLS                 FALSE
#define GDISP_HARDWARE_BITFILLS              FALSE
#define GDISP_HARDWARE_SCROLL                FALSE
#define GDISP_HARDWARE_QUERY                 FALSE
#define GDISP_HARDWARE_CLIP                  FALSE
#define GDISP_LLD_PIXELFORMAT			GDISP_PIXELFORMAT_RGB888

#endif	/* GFX_USE_GDISP */

#endif	/* _GDISP_LLD_CONFIG_H */
