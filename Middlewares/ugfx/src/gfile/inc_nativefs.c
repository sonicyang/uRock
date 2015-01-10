/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * This file is included by src/gfile/gfile.c
 */

/********************************************************
 * The native file-system
 ********************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>

static GFILE NativeStdIn;
static GFILE NativeStdOut;
static GFILE NativeStdErr;

static bool_t NativeDel(const char *fname);
static bool_t NativeExists(const char *fname);
static long int	NativeFilesize(const char *fname);
static bool_t NativeRen(const char *oldname, const char *newname);
static bool_t NativeOpen(GFILE *f, const char *fname);
static void NativeClose(GFILE *f);
static int NativeRead(GFILE *f, void *buf, int size);
static int NativeWrite(GFILE *f, const void *buf, int size);
static bool_t NativeSetpos(GFILE *f, long int pos);
static long int NativeGetsize(GFILE *f);
static bool_t NativeEof(GFILE *f);

static const GFILEVMT FsNativeVMT = {
	GFILE_CHAINHEAD,									// next
	#if defined(WIN32) || GFX_USE_OS_WIN32
		GFSFLG_TEXTMODES|
	#else
		GFSFLG_CASESENSITIVE|
	#endif
	GFSFLG_WRITEABLE|GFSFLG_SEEKABLE|GFSFLG_FAST,		// flags
	'N',												// prefix
	NativeDel, NativeExists, NativeFilesize, NativeRen,
	NativeOpen, NativeClose, NativeRead, NativeWrite,
	NativeSetpos, NativeGetsize, NativeEof,
};
#undef GFILE_CHAINHEAD
#define GFILE_CHAINHEAD		&FsNativeVMT

static void flags2mode(char *buf, uint16_t flags) {
	if (flags & GFILEFLG_MUSTEXIST)
		*buf = 'r';
	else if (flags & GFILEFLG_APPEND)
		*buf = 'a';
	else
		*buf = 'w';
	buf++;
	if ((flags & (GFILEFLG_READ|GFILEFLG_WRITE)) == (GFILEFLG_READ|GFILEFLG_WRITE))
		*buf++ = '+';
	if (flags & GFILEFLG_BINARY)
		*buf++ = 'b';
	if (flags & GFILEFLG_MUSTNOTEXIST)
		*buf++ = 'x';
	*buf++ = 0;
}

static bool_t NativeDel(const char *fname)							{ return remove(fname) ? FALSE : TRUE; }
static void NativeClose(GFILE *f)									{ fclose((FILE *)f->obj); }
static int NativeRead(GFILE *f, void *buf, int size)				{ return fread(buf, 1, size, (FILE *)f->obj); }
static int NativeWrite(GFILE *f, const void *buf, int size)			{ return fwrite(buf, 1, size, (FILE *)f->obj); }
static bool_t NativeSetpos(GFILE *f, long int pos)					{ return fseek((FILE *)f->obj, pos, SEEK_SET) ?  FALSE : TRUE; }
static bool_t NativeEof(GFILE *f)									{ return feof((FILE *)f->obj) ? TRUE : FALSE; }
static bool_t NativeRen(const char *oldname, const char *newname)	{ return rename(oldname, newname) ? FALSE : TRUE; }
static bool_t NativeExists(const char *fname) {
	// We define access this way so we don't have to include <unistd.h> which may
	//	(and does under windows) contain conflicting definitions for types such as uint16_t.
	extern int access(const char *pathname, int mode);
	return access(fname, 0) ? FALSE : TRUE;
}
static long int	NativeFilesize(const char *fname) {
	struct stat st;
	if (stat(fname, &st)) return -1;
	return st.st_size;
}
static bool_t NativeOpen(GFILE *f, const char *fname) {
	FILE *fd;
	char mode[5];

	flags2mode(mode, f->flags);
	if (!(fd = fopen(fname, mode)))
		return FALSE;
	f->obj = (void *)fd;
	return TRUE;
}
static long int NativeGetsize(GFILE *f) {
	struct stat st;
	if (fstat(fileno((FILE *)f->obj), &st)) return -1;
	return st.st_size;
}
