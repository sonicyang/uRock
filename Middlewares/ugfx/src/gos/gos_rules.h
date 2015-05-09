/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gos/gos_rules.h
 * @brief   GOS safety rules header file.
 *
 * @addtogroup GOS
 * @{
 */

#ifndef _GOS_RULES_H
#define _GOS_RULES_H

#if !GFX_USE_OS_CHIBIOS && !GFX_USE_OS_WIN32 && !GFX_USE_OS_LINUX && !GFX_USE_OS_OSX && !GFX_USE_OS_RAW32 && !GFX_USE_OS_FREERTOS && !GFX_USE_OS_ECOS && !GFX_USE_OS_RAWRTOS && !GFX_USE_OS_ARDUINO
	#if GFX_DISPLAY_RULE_WARNINGS
		#warning "GOS: No Operating System has been defined. ChibiOS (GFX_USE_OS_CHIBIOS) has been turned on for you."
	#endif
	#undef GFX_USE_OS_CHIBIOS
	#define GFX_USE_OS_CHIBIOS	TRUE
#endif

#if GFX_USE_OS_CHIBIOS + GFX_USE_OS_WIN32 + GFX_USE_OS_LINUX + GFX_USE_OS_OSX + GFX_USE_OS_RAW32 + GFX_USE_OS_FREERTOS + GFX_USE_OS_ECOS + GFX_USE_OS_RAWRTOS + GFX_USE_OS_ARDUINO != 1 * TRUE
	#error "GOS: More than one operation system has been defined as TRUE."
#endif

#if GFX_FREERTOS_USE_TRACE && !GFX_USE_OS_FREERTOS
 	#error "GOS: GFX_FREERTOS_USE_TRACE is only available for the FreeRTOS port."
#endif

#endif /* _GOS_RULES_H */
/** @} */
