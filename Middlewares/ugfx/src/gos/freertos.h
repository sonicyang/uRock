/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gos/freertos.h
 * @brief   GOS - Operating System Support header file for FreeRTOS.
 */

#ifndef _GOS_FREERTOS_H
#define _GOS_FREERTOS_H

#if GFX_USE_OS_FREERTOS

#include "../../../Third_Party/FreeRTOS/Source/include/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "../../../Third_Party/FreeRTOS/Source/include/semphr.h"
#include "../../../Third_Party/FreeRTOS/Source/include/task.h"

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

/**
 * bool_t,
 * int8_t, uint8_t,
 * int16_t, uint16_t,
 * int32_t, uint32_t,
 * size_t
 * TRUE, FALSE
 * are already defined by FreeRTOS
 */
#define TIME_IMMEDIATE		0
#define TIME_INFINITE		((delaytime_t)-1)
typedef int8_t				bool_t;
typedef uint32_t			delaytime_t;
typedef portTickType		systemticks_t;
typedef int32_t				semcount_t;
typedef void				threadreturn_t;
typedef portBASE_TYPE		threadpriority_t;

#define MAX_SEMAPHORE_COUNT	((semcount_t)(((unsigned long)((semcount_t)(-1))) >> 1))
#define LOW_PRIORITY		0
#define NORMAL_PRIORITY		configMAX_PRIORITIES/2
#define HIGH_PRIORITY		configMAX_PRIORITIES-1

/* FreeRTOS will allocate the stack when creating the thread, so pass the size instead of a working area */
#define DECLARE_THREAD_STACK(name, sz)			size_t *name = (size_t *)sz
#define DECLARE_THREAD_FUNCTION(fnName, param)	threadreturn_t fnName(void *param)
portTickType MS2ST(portTickType ms);

typedef struct {
	xSemaphoreHandle	sem;
	semcount_t			limit;
	semcount_t  		counter;
} gfxSem;

typedef xSemaphoreHandle		gfxMutex;
typedef xTaskHandle*				gfxThreadHandle;

/*===========================================================================*/
/* Function declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

#define gfxHalt(msg)				{}
#define gfxExit()					{}
#define gfxAlloc(sz)				pvPortMalloc(sz)
#define gfxFree(ptr)				vPortFree(ptr)
#define gfxYield()					taskYIELD()
#define gfxSystemTicks()			xTaskGetTickCount()
#define gfxMillisecondsToTicks(ms)	MS2ST(ms)
#define gfxSystemLock()				{}
#define gfxSystemUnlock()			{}

void gfxMutexInit(xSemaphoreHandle* s);
#define gfxMutexDestroy(pmutex)		vSemaphoreDelete(*pmutex)
#define gfxMutexEnter(pmutex)		xSemaphoreTake(*pmutex,portMAX_DELAY)
#define gfxMutexExit(pmutex)		xSemaphoreGive(*pmutex)

void *gfxRealloc(void *ptr, size_t oldsz, size_t newsz);
void gfxSleepMilliseconds(delaytime_t ms);
void gfxSleepMicroseconds(delaytime_t ms);

void gfxSemInit(gfxSem* psem, semcount_t val, semcount_t limit);
void gfxSemDestroy(gfxSem* psem);
bool_t gfxSemWait(gfxSem* psem, delaytime_t ms);
bool_t gfxSemWaitI(gfxSem* psem);
void gfxSemSignal(gfxSem* psem);
void gfxSemSignalI(gfxSem* psem);
#define gfxSemCounterI(psem)		((psem)->counter)
#define gfxSemCounter(psem)			((psem)->counter)
gfxThreadHandle gfxThreadCreate(void *stackarea, size_t stacksz, threadpriority_t prio, DECLARE_THREAD_FUNCTION((*fn),p), void *param);

#define gfxThreadWait(thread)		{} // never used, not imlpemented
#define gfxThreadMe()				{} // never used, not implemented
#define gfxThreadClose(thread)		{}

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_OS_FREERTOS */
#endif /* _GOS_CHIBIOS_H */
