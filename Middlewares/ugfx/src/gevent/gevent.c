/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gevent/gevent.c
 * @brief   GEVENT Driver code.
 *
 * @addtogroup GEVENT
 * @{
 */
#include "gfx.h"

#if GFX_USE_GEVENT || defined(__DOXYGEN__)

#if GEVENT_ASSERT_NO_RESOURCE
	#define GEVENT_ASSERT(x)		assert(x)
#else
	#define GEVENT_ASSERT(x)
#endif

/* This mutex protects access to our tables */
static gfxMutex	geventMutex;

/* Our table of listener/source pairs */
static GSourceListener		Assignments[GEVENT_MAX_SOURCE_LISTENERS];

/* Loop through the assignment table deleting this listener/source pair. */
/*	Null is treated as a wildcard. */
static void deleteAssignments(GListener *pl, GSourceHandle gsh) {
	GSourceListener *psl;

	for(psl = Assignments; psl < Assignments+GEVENT_MAX_SOURCE_LISTENERS; psl++) {
		if ((!pl || psl->pListener == pl) && (!gsh || psl->pSource == gsh)) {
			if (gfxSemCounter(&psl->pListener->waitqueue) < 0) {
				gfxSemWait(&psl->pListener->eventlock, TIME_INFINITE);	// Obtain the buffer lock
				psl->pListener->event.type = GEVENT_EXIT;				// Set up the EXIT event
				gfxSemSignal(&psl->pListener->waitqueue);				// Wake up the listener
				gfxSemSignal(&psl->pListener->eventlock);				// Release the buffer lock
			}
			psl->pListener = 0;
		}
	}
}

void _geventInit(void)
{
	gfxMutexInit(&geventMutex);
}

void _geventDeinit(void)
{
	gfxMutexDestroy(&geventMutex);	
}

void geventListenerInit(GListener *pl) {
	gfxSemInit(&pl->waitqueue, 0, MAX_SEMAPHORE_COUNT);		// Next wait'er will block
	gfxSemInit(&pl->eventlock, 1, 1);						// Only one thread at a time looking at the event buffer
	pl->callback = 0;										// No callback active
	pl->event.type = GEVENT_NULL;							// Always safety
}

bool_t geventAttachSource(GListener *pl, GSourceHandle gsh, unsigned flags) {
	GSourceListener *psl, *pslfree;

	// Safety first
	if (!pl || !gsh) {
		GEVENT_ASSERT(FALSE);
		return FALSE;
	}

	gfxMutexEnter(&geventMutex);

	// Check if this pair is already in the table (scan for a free slot at the same time)
	pslfree = 0;
	for(psl = Assignments; psl < Assignments+GEVENT_MAX_SOURCE_LISTENERS; psl++) {
		
		if (pl == psl->pListener && gsh == psl->pSource) {
			// Just update the flags
			gfxSemWait(&pl->eventlock, TIME_INFINITE);		// Safety first - just in case a source is using it
			psl->listenflags = flags;
			gfxSemSignal(&pl->eventlock);			// Release this lock
			gfxMutexExit(&geventMutex);
			return TRUE;
		}
		if (!pslfree && !psl->pListener)
			pslfree = psl;
	}
	
	// A free slot was found - allocate it
	if (pslfree) {
		pslfree->pListener = pl;
		pslfree->pSource = gsh;
		pslfree->listenflags = flags;
		pslfree->srcflags = 0;
	}
	gfxMutexExit(&geventMutex);
	GEVENT_ASSERT(pslfree != 0);
	return pslfree != 0;
}

void geventDetachSource(GListener *pl, GSourceHandle gsh) {
	if (pl) {
		gfxMutexEnter(&geventMutex);
		deleteAssignments(pl, gsh);
		if (!gsh && gfxSemCounter(&pl->waitqueue) < 0) {
			gfxSemWait(&pl->eventlock, TIME_INFINITE);		// Obtain the buffer lock
			pl->event.type = GEVENT_EXIT;					// Set up the EXIT event
			gfxSemSignal(&pl->waitqueue);					// Wake up the listener
			gfxSemSignal(&pl->eventlock);					// Release the buffer lock
		}
		gfxMutexExit(&geventMutex);
	}
}

GEvent *geventEventWait(GListener *pl, delaytime_t timeout) {
	if (pl->callback || gfxSemCounter(&pl->waitqueue) < 0)
		return 0;
	return gfxSemWait(&pl->waitqueue, timeout) ? &pl->event : 0;
}

void geventRegisterCallback(GListener *pl, GEventCallbackFn fn, void *param) {
	if (pl) {
		gfxMutexEnter(&geventMutex);
		gfxSemWait(&pl->eventlock, TIME_INFINITE);			// Obtain the buffer lock
		pl->param = param;									// Set the param
		pl->callback = fn;									// Set the callback function
		if (gfxSemCounter(&pl->waitqueue) < 0) {
			pl->event.type = GEVENT_EXIT;			// Set up the EXIT event
			gfxSemSignal(&pl->waitqueue);			// Wake up the listener
		}
		gfxSemSignal(&pl->eventlock);				// Release the buffer lock
		gfxMutexExit(&geventMutex);
	}
}

GSourceListener *geventGetSourceListener(GSourceHandle gsh, GSourceListener *lastlr) {
	GSourceListener *psl;

	// Safety first
	if (!gsh)
		return 0;

	gfxMutexEnter(&geventMutex);

	// Unlock the last listener event buffer
	if (lastlr)
		gfxSemSignal(&lastlr->pListener->eventlock);
		
	// Loop through the table looking for attachments to this source
	for(psl = lastlr ? (lastlr+1) : Assignments; psl < Assignments+GEVENT_MAX_SOURCE_LISTENERS; psl++) {
		if (gsh == psl->pSource) {
			gfxSemWait(&psl->pListener->eventlock, TIME_INFINITE);		// Obtain a lock on the listener event buffer
			gfxMutexExit(&geventMutex);
			return psl;
		}
	}
	gfxMutexExit(&geventMutex);
	return 0;
}

GEvent *geventGetEventBuffer(GSourceListener *psl) {
	// We already know we have the event lock
	return &psl->pListener->callback || gfxSemCounter(&psl->pListener->waitqueue) < 0 ? &psl->pListener->event : 0;
}

void geventSendEvent(GSourceListener *psl) {
	gfxMutexEnter(&geventMutex);
	if (psl->pListener->callback) {				// This test needs to be taken inside the mutex
		gfxMutexExit(&geventMutex);
		// We already know we have the event lock
		psl->pListener->callback(psl->pListener->param, &psl->pListener->event);

	} else {
		// Wake up the listener
		if (gfxSemCounter(&psl->pListener->waitqueue) <= 0)
			gfxSemSignal(&psl->pListener->waitqueue);
		gfxMutexExit(&geventMutex);
	}
}

void geventDetachSourceListeners(GSourceHandle gsh) {
	gfxMutexEnter(&geventMutex);
	deleteAssignments(0, gsh);
	gfxMutexExit(&geventMutex);
}

#endif /* GFX_USE_GEVENT */
/** @} */
