#ifndef __OSDEBUG_H__
#define __OSDEBUG_H__

void osDbgPrintf(const char * fmt, ...);

#define DBGOUT osDbgPrintf

#endif
