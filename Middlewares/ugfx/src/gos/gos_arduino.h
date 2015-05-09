/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * The arduino GOS implementation is similar to the bare metal raw32 gos implementation.
 * It uses cooperative multi-tasking. Be careful
 * 	when writing device drivers not to disturb the assumptions this creates by performing
 * 	call-backs to uGFX code unless you define the INTERRUPTS_OFF() and INTERRUPTS_ON() macros.
 * 	It still requires some C runtime library support...
 * 		enough startup to initialise the stack, interrupts, static data etc and call main().
 * 		setjmp() and longjmp()			- for threading
 * 		memcpy()						- for heap and threading
 * 		malloc(), realloc and free()	- if GOS_RAW_HEAP_SIZE == 0
 *
 * 	You must also define the following routines in your own code so that timing functions will work...
 * 		systemticks_t gfxSystemTicks(void);
 *		systemticks_t gfxMillisecondsToTicks(delaytime_t ms);
 */
#ifndef _GOS_ARDUINO_H
#define _GOS_ARDUINO_H

#if GFX_USE_OS_ARDUINO

#include <Arduino.h>

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

typedef bool	bool_t;

#if 0
	// Already defined by Arduino
	typedef signed char		int8_t;
	typedef unsigned char	uint8_t;
	typedef signed short	int16_t;
	typedef unsigned short	uint16_t;
	typedef signed int		int32_t;
	typedef unsigned int	uint32_t;
	typedef uint32_t		size_t;
#endif

typedef uint32_t		delaytime_t;
typedef uint32_t		systemticks_t;
typedef short			semcount_t;
typedef int				threadreturn_t;
typedef int				threadpriority_t;

#define DECLARE_THREAD_FUNCTION(fnName, param)	threadreturn_t fnName(void *param)
#define DECLARE_THREAD_STACK(name, sz)			uint8_t name[sz];

#define TIME_IMMEDIATE				0
#define TIME_INFINITE				((delaytime_t)-1)
#define MAX_SEMAPHORE_COUNT			0x7FFF
#define LOW_PRIORITY				0
#define NORMAL_PRIORITY				1
#define HIGH_PRIORITY				2

typedef struct {
	semcount_t		cnt;
	semcount_t		limit;
} gfxSem;

typedef uint32_t		gfxMutex;
typedef void *			gfxThreadHandle;

#define gfxThreadClose(thread)
#define gfxMutexDestroy(pmutex)
#define gfxSemDestroy(psem)
#define gfxSemCounter(psem)			((psem)->cnt)
#define gfxSemCounterI(psem)		((psem)->cnt)

#define gfxSystemTicks()			millis()
#define gfxMillisecondsToTicks(ms)	(ms)

#ifdef __cplusplus
extern "C" {
#endif

	void gfxHalt(const char *msg);
	void gfxExit(void);
	void *gfxAlloc(size_t sz);
	void *gfxRealloc(void *ptr, size_t oldsz, size_t newsz);
	void gfxFree(void *ptr);
	void gfxYield(void);
	void gfxSleepMilliseconds(delaytime_t ms);
	void gfxSleepMicroseconds(delaytime_t ms);
	//systemticks_t gfxSystemTicks(void);
	//systemticks_t gfxMillisecondsToTicks(delaytime_t ms);
	void gfxSystemLock(void);
	void gfxSystemUnlock(void);
	void gfxMutexInit(gfxMutex *pmutex);
	void gfxMutexEnter(gfxMutex *pmutex);
	void gfxMutexExit(gfxMutex *pmutex);
	void gfxSemInit(gfxSem *psem, semcount_t val, semcount_t limit);
	bool_t gfxSemWait(gfxSem *psem, delaytime_t ms);
	bool_t gfxSemWaitI(gfxSem *psem);
	void gfxSemSignal(gfxSem *psem);
	void gfxSemSignalI(gfxSem *psem);
	gfxThreadHandle gfxThreadCreate(void *stackarea, size_t stacksz, threadpriority_t prio, DECLARE_THREAD_FUNCTION((*fn),p), void *param);
	threadreturn_t gfxThreadWait(gfxThreadHandle thread);
	gfxThreadHandle gfxThreadMe(void);

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_OS_ARDUINO */
#endif /* _GOS_ARDUINO_H */
