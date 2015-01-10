/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/ginput.c
 * @brief   GINPUT subsystem common code.
 *
 * @addtogroup GINPUT
 * @{
 */
#include "gfx.h"

#if GFX_USE_GINPUT

void _ginputInit(void)
{
	/* ToDo */

	/**
	 * This should really call an init routine for each ginput sub-system.
	 * Maybe we'll do this later.
	 */
}

void _ginputDeinit(void)
{

}

#endif /* GFX_USE_GINPUT */
/** @} */

