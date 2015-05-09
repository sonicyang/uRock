/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/sys_rules.h
 * @brief   GINPUT safety rules header file.
 *
 * @addtogroup GINPUT
 * @{
 */

#ifndef _GINPUT_RULES_H
#define _GINPUT_RULES_H

#if GFX_USE_GINPUT
	#if !GFX_USE_GEVENT
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GINPUT: GFX_USE_GEVENT is required if GFX_USE_GINPUT is TRUE. It has been turned on for you."
		#endif
		#undef GFX_USE_GEVENT
		#define	GFX_USE_GEVENT		TRUE
	#endif
	#if !GFX_USE_GTIMER
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GINPUT: GFX_USE_GTIMER is required if GFX_USE_GINPUT is TRUE. It has been turned on for you."
		#endif
		#undef GFX_USE_GTIMER
		#define	GFX_USE_GTIMER		TRUE
	#endif
#endif

#endif /* _GINPUT_RULES_H */
/** @} */
