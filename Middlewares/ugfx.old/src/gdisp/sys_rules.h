/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gdisp/sys_rules.h
 * @brief   GDISP safety rules header file.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_RULES_H
#define _GDISP_RULES_H

#if GFX_USE_GDISP
	#if GDISP_TOTAL_CONTROLLERS > 1
		#ifndef GDISP_CONTROLLER_LIST
			#error "GDISP Multiple Controllers: You must specify a value for GDISP_CONTROLLER_LIST"
		#endif
		#ifndef GDISP_CONTROLLER_DISPLAYS
			#error "GDISP Multiple Controllers: You must specify a value for GDISP_CONTROLLER_DISPLAYS"
		#endif
		#ifndef GDISP_PIXELFORMAT
			#error "GDISP Multiple Controllers: You must specify a value for GDISP_PIXELFORMAT"
		#endif
	#endif
	#if GDISP_NEED_AUTOFLUSH && GDISP_NEED_TIMERFLUSH
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GDISP: Both GDISP_NEED_AUTOFLUSH and GDISP_NEED_TIMERFLUSH has been set. GDISP_NEED_TIMERFLUSH has disabled for you."
		#endif
		#undef GDISP_NEED_TIMERFLUSH
		#define GDISP_NEED_TIMERFLUSH		FALSE
	#endif
	#if GDISP_NEED_TIMERFLUSH
		#if GDISP_NEED_TIMERFLUSH < 50 || GDISP_NEED_TIMERFLUSH > 1200
			#error "GDISP: GDISP_NEED_TIMERFLUSH has been set to an invalid value (FALSE, 50-1200)."
		#endif
		#if !GFX_USE_GTIMER
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GDISP: GDISP_NEED_TIMERFLUSH has been set but GFX_USE_GTIMER has not been set. It has been turned on for you."
			#endif
			#undef GFX_USE_GTIMER
			#define GFX_USE_GTIMER				TRUE
			#undef GDISP_NEED_MULTITHREAD
			#define GDISP_NEED_MULTITHREAD		TRUE
		#endif
	#endif
	#if GDISP_NEED_ANTIALIAS && !GDISP_NEED_PIXELREAD
		#if GDISP_HARDWARE_PIXELREAD
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GDISP: GDISP_NEED_ANTIALIAS has been set but GDISP_NEED_PIXELREAD has not. It has been turned on for you."
			#endif
			#undef GDISP_NEED_PIXELREAD
			#define GDISP_NEED_PIXELREAD	TRUE
		#else
			#if GFX_DISPLAY_RULE_WARNINGS
                //Known -> Supressed
				//#warning "GDISP: GDISP_NEED_ANTIALIAS has been set but your hardware does not support reading back pixels. Anti-aliasing will only occur for filled characters."
			#endif
		#endif
	#endif
	#if (defined(GDISP_INCLUDE_FONT_SMALL) && GDISP_INCLUDE_FONT_SMALL) || (defined(GDISP_INCLUDE_FONT_LARGER) && GDISP_INCLUDE_FONT_LARGER)
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GDISP: An old font (Small or Larger) has been defined. A single default font of UI2 has been added instead."
			#warning "GDISP: Please see <$(GFXLIB)/include/gdisp/fonts/fonts.h> for a list of available font names."
		#endif
		#undef GDISP_INCLUDE_FONT_UI2
		#define GDISP_INCLUDE_FONT_UI2		TRUE
	#endif
	#if GDISP_NEED_IMAGE
		#if !GFX_USE_GFILE
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GDISP: GFX_USE_GFILE is required when GDISP_NEED_IMAGE is TRUE. It has been turned on for you."
			#endif
			#undef GFX_USE_GFILE
			#define GFX_USE_GFILE	TRUE
		#endif
	#endif
#endif

#endif /* _GDISP_RULES_H */
/** @} */
