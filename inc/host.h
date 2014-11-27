#ifndef HOST_H
#define HOST_H
#include <string.h>
#include <stdarg.h>


/*
 *Reference: http://albert-oma.blogspot.tw/2012/04/semihosting.html
 */
enum HOST_SYSCALL{
	SYS_OPEN=0x01,
	SYS_CLOSE,
	SYS_WRITEC,
	SYS_WRITE0,
	SYS_WRITE,
	SYS_READ,
	SYS_READC,
	SYS_ISERROR,
	SYS_ISTTY,
	SYS_SEEK,
	SYS_FLEN=0xC,
	SYS_TMPNAM,
	SYS_REMOVE,
	SYS_RENAME,
	SYS_CLOCK,
	SYS_TIME,
	SYS_SYSTEM,
	SYS_ERRNO,
	SYS_GET_CMDLINE=0x15,
	SYS_HEAPINFO,
	SYS_ELAPSED=0x30,
	SYS_TICKFREQ
};

int host_call(enum HOST_SYSCALL, void *argv) __attribute__((naked));

int host_system(va_list v1);
int host_open(va_list v1);
int host_close(va_list v1);
int host_write(va_list v1);

int host_action(enum HOST_SYSCALL action, ...);

#endif 
