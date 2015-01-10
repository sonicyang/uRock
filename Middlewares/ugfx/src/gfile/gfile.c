/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gfile/gfile.c
 * @brief   GFILE code.
 *
 */

#define GFILE_IMPLEMENTATION

#include "gfx.h"

#if GFX_USE_GFILE

struct GFILE {
	const struct GFILEVMT *	vmt;
	uint16_t				flags;
		#define	GFILEFLG_OPEN			0x0001		// File is open
		#define	GFILEFLG_READ			0x0002		// Read the file
		#define	GFILEFLG_WRITE			0x0004		// Write the file
		#define	GFILEFLG_APPEND			0x0008		// Append on each write
		#define GFILEFLG_BINARY			0x0010		// Treat as a binary file
		#define	GFILEFLG_DELONCLOSE		0x0020		// Delete on close
		#define	GFILEFLG_CANSEEK		0x0040		// Seek operations are valid
		#define GFILEFLG_FAILONBLOCK	0x0080		// Fail on a blocking call
		#define GFILEFLG_MUSTEXIST		0x0100		// On open file must exist
		#define GFILEFLG_MUSTNOTEXIST	0x0200		// On open file must not exist
		#define GFILEFLG_TRUNC			0x0400		// On open truncate the file
	void *					obj;
	long int				pos;
};

typedef struct GFILEVMT {
	const struct GFILEVMT *	next;
	uint8_t					flags;
		#define GFSFLG_WRITEABLE		0x0001
		#define GFSFLG_CASESENSITIVE	0x0002
		#define GFSFLG_SEEKABLE			0x0004
		#define GFSFLG_FAST				0x0010
		#define GFSFLG_SMALL			0x0020
		#define GFSFLG_TEXTMODES		0x0040
	char					prefix;
	bool_t		(*del)		(const char *fname);
	bool_t		(*exists)	(const char *fname);
	long int	(*filesize)	(const char *fname);
	bool_t		(*ren)		(const char *oldname, const char *newname);
	bool_t		(*open)		(GFILE *f, const char *fname);
	void		(*close)	(GFILE *f);
	int			(*read)		(GFILE *f, void *buf, int size);
	int			(*write)	(GFILE *f, const void *buf, int size);
	bool_t		(*setpos)	(GFILE *f, long int pos);
	long int	(*getsize)	(GFILE *f);
	bool_t		(*eof)		(GFILE *f);
} GFILEVMT;

// The chain of FileSystems
#define GFILE_CHAINHEAD		0

// The table of GFILE's
static GFILE gfileArr[GFILE_MAX_GFILES];
GFILE *gfileStdIn;
GFILE *gfileStdOut;
GFILE *gfileStdErr;

/**
 * The order of the file-systems below determines the order
 * that they are searched to find a file.
 * The last defined is the first searched.
 */

/********************************************************
 * The ChibiOS BaseFileStream VMT
 ********************************************************/
#if GFILE_NEED_CHIBIOSFS && GFX_USE_OS_CHIBIOS
	#include "src/gfile/inc_chibiosfs.c"
#endif

/********************************************************
 * The Memory Pointer VMT
 ********************************************************/
#if GFILE_NEED_MEMFS
	#include "src/gfile/inc_memfs.c"
#endif

/********************************************************
 * The RAM file-system VMT
 ********************************************************/
#if GFILE_NEED_RAMFS
	#include "src/gfile/inc_ramfs.c"
#endif

/********************************************************
 * The FAT file-system VMT
 ********************************************************/
#ifndef GFILE_NEED_FATFS
	#include "src/gfile/inc_fatfs.c"
#endif

/********************************************************
 * The native file-system
 ********************************************************/
#if GFILE_NEED_NATIVEFS
	#include "src/gfile/inc_nativefs.c"
#endif

/********************************************************
 * The ROM file-system VMT
 ********************************************************/
#if GFILE_NEED_ROMFS
	#include "src/gfile/inc_romfs.c"
#endif

/********************************************************
 * IO routines
 ********************************************************/

/**
 * The chain of file systems.
 */
static const GFILEVMT const * FsChain = GFILE_CHAINHEAD;

/**
 * The init routine
 */
void _gfileInit(void) {
	#if GFILE_NEED_NATIVEFS
		NativeStdIn.flags = GFILEFLG_OPEN|GFILEFLG_READ;
		NativeStdIn.vmt = &FsNativeVMT;
		NativeStdIn.obj = (void *)stdin;
		NativeStdIn.pos = 0;
		gfileStdIn = &NativeStdIn;
		NativeStdOut.flags = GFILEFLG_OPEN|GFILEFLG_WRITE|GFILEFLG_APPEND;
		NativeStdOut.vmt = &FsNativeVMT;
		NativeStdOut.obj = (void *)stdout;
		NativeStdOut.pos = 0;
		gfileStdOut = &NativeStdOut;
		NativeStdErr.flags = GFILEFLG_OPEN|GFILEFLG_WRITE|GFILEFLG_APPEND;
		NativeStdErr.vmt = &FsNativeVMT;
		NativeStdErr.obj = (void *)stderr;
		NativeStdErr.pos = 0;
		gfileStdErr = &NativeStdErr;
	#endif
}

void _gfileDeinit(void)
{
	/* ToDo */
}

bool_t gfileExists(const char *fname) {
	const GFILEVMT *p;

	#if GFILE_ALLOW_DEVICESPECIFIC
		if (fname[0] && fname[1] == '|') {
			for(p = FsChain; p; p = p->next) {
				if (p->prefix == fname[0])
					return p->exists && p->exists(fname+2);
			}
			return FALSE;
		}
	#endif

	for(p = FsChain; p; p = p->next) {
		if (p->exists && p->exists(fname))
			return TRUE;
	}
	return FALSE;
}

bool_t gfileDelete(const char *fname) {
	const GFILEVMT *p;

	#if GFILE_ALLOW_DEVICESPECIFIC
		if (fname[0] && fname[1] == '|') {
			for(p = FsChain; p; p = p->next) {
				if (p->prefix == fname[0])
					return p->del && p->del(fname+2);
			}
			return FALSE;
		}
	#endif

	for(p = FsChain; p; p = p->next) {
		if (p->del && p->del(fname))
			return TRUE;
	}
	return FALSE;
}

long int gfileGetFilesize(const char *fname) {
	const GFILEVMT *p;
	long int res;

	#if GFILE_ALLOW_DEVICESPECIFIC
		if (fname[0] && fname[1] == '|') {
			for(p = FsChain; p; p = p->next) {
				if (p->prefix == fname[0])
					return p->filesize ? p->filesize(fname+2) : -1;
			}
			return -1;
		}
	#endif

	for(p = FsChain; p; p = p->next) {
		if (p->filesize && (res = p->filesize(fname)) != -1)
			return res;
	}
	return -1;
}

bool_t gfileRename(const char *oldname, const char *newname) {
	const GFILEVMT *p;

	#if GFILE_ALLOW_DEVICESPECIFIC
		if ((oldname[0] && oldname[1] == '|') || (newname[0] && newname[1] == '|')) {
			char ch;

			if (oldname[0] && oldname[1] == '|') {
				ch = oldname[0];
				oldname += 2;
				if (newname[0] && newname[1] == '|') {
					if (newname[0] != ch)
						// Both oldname and newname are fs specific but different ones.
						return FALSE;
					newname += 2;
				}
			} else {
				ch = newname[0];
				newname += 2;
			}
			for(p = FsChain; p; p = p->next) {
				if (p->prefix == ch)
					return p->ren && p->ren(oldname, newname);
			}
			return FALSE;
		}
	#endif

	for(p = FsChain; p; p = p->next) {
		if (p->ren && p->ren(oldname,newname))
			return TRUE;
	}
	return FALSE;
}

static uint16_t mode2flags(const char *mode) {
	uint16_t	flags;

	switch(mode[0]) {
	case 'r':
		flags = GFILEFLG_READ|GFILEFLG_MUSTEXIST;
		while (*++mode) {
			switch(mode[0]) {
			case '+':	flags |= GFILEFLG_WRITE;		break;
			case 'b':	flags |= GFILEFLG_BINARY;		break;
			}
		}
		return flags;
	case 'w':
		flags = GFILEFLG_WRITE|GFILEFLG_TRUNC;
		while (*++mode) {
			switch(mode[0]) {
			case '+':	flags |= GFILEFLG_READ;			break;
			case 'b':	flags |= GFILEFLG_BINARY;		break;
			case 'x':	flags |= GFILEFLG_MUSTNOTEXIST;	break;
			}
		}
		return flags;
	case 'a':
		flags = GFILEFLG_WRITE|GFILEFLG_APPEND;
		while (*++mode) {
			switch(mode[0]) {
			case '+':	flags |= GFILEFLG_READ;			break;
			case 'b':	flags |= GFILEFLG_BINARY;		break;
			case 'x':	flags |= GFILEFLG_MUSTNOTEXIST;	break;
			}
		}
		return flags;
	}
	return 0;
}

static bool_t testopen(const GFILEVMT *p, GFILE *f, const char *fname) {
	// If we want write but the fs doesn't allow it then return
	if ((f->flags & GFILEFLG_WRITE) && !(p->flags & GFSFLG_WRITEABLE))
		return FALSE;

	// Try to open
	if (!p->open || !p->open(f, fname))
		return FALSE;

	// File is open - fill in all the details
	f->vmt = p;
	f->pos = 0;
	f->flags |= GFILEFLG_OPEN;
	if (p->flags & GFSFLG_SEEKABLE)
		f->flags |= GFILEFLG_CANSEEK;
	return TRUE;
}

GFILE *gfileOpen(const char *fname, const char *mode) {
	uint16_t		flags;
	GFILE *			f;
	const GFILEVMT *p;

	// Get the requested mode
	if (!(flags = mode2flags(mode)))
		return 0;

	#if GFILE_ALLOW_DEVICESPECIFIC
		if (fname[0] && fname[1] == '|') {
			// First find an available GFILE slot.
			for (f = gfileArr; f < &gfileArr[GFILE_MAX_GFILES]; f++) {
				if (!(f->flags & GFILEFLG_OPEN)) {
					// Try to open the file
					f->flags = flags;
					for(p = FsChain; p; p = p->next) {
						if (p->prefix == fname[0])
							return testopen(p, f, fname+2) ? f : 0;
					}
					// File not found
					break;
				}
			}

			// No available slot
			return 0;
		}
	#endif

	// First find an available GFILE slot.
	for (f = gfileArr; f < &gfileArr[GFILE_MAX_GFILES]; f++) {
		if (!(f->flags & GFILEFLG_OPEN)) {

			// Try to open the file
			f->flags = flags;
			for(p = FsChain; p; p = p->next) {
				if (testopen(p, f, fname))
					return f;
			}
			// File not found
			break;
		}
	}

	// No available slot
	return 0;
}

#if GFILE_NEED_CHIBIOSFS && GFX_USE_OS_CHIBIOS
	GFILE *		gfileOpenBaseFileStream(void *BaseFileStreamPtr, const char *mode) {
		GFILE *			f;

		// First find an available GFILE slot.
		for (f = gfileArr; f < &gfileArr[GFILE_MAX_GFILES]; f++) {
			if (!(f->flags & GFILEFLG_OPEN)) {
				// Get the flags
				if (!(f->flags = mode2flags(mode)))
					return 0;

				// If we want write but the fs doesn't allow it then return
				if ((f->flags & GFILEFLG_WRITE) && !(FsCHIBIOSVMT.flags & GFSFLG_WRITEABLE))
					return 0;

				// File is open - fill in all the details
				f->vmt = &FsCHIBIOSVMT;
				f->obj = BaseFileStreamPtr;
				f->pos = 0;
				f->flags |= GFILEFLG_OPEN|GFILEFLG_CANSEEK;
				return f;
			}
		}

		// No available slot
		return 0;
	}
#endif

#if GFILE_NEED_MEMFS
	GFILE *		gfileOpenMemory(void *memptr, const char *mode) {
		GFILE *			f;

		// First find an available GFILE slot.
		for (f = gfileArr; f < &gfileArr[GFILE_MAX_GFILES]; f++) {
			if (!(f->flags & GFILEFLG_OPEN)) {
				// Get the flags
				if (!(f->flags = mode2flags(mode)))
					return 0;

				// If we want write but the fs doesn't allow it then return
				if ((f->flags & GFILEFLG_WRITE) && !(FsMemVMT.flags & GFSFLG_WRITEABLE))
					return 0;

				// File is open - fill in all the details
				f->vmt = &FsMemVMT;
				f->obj = memptr;
				f->pos = 0;
				f->flags |= GFILEFLG_OPEN|GFILEFLG_CANSEEK;
				return f;
			}
		}

		// No available slot
		return 0;
	}
#endif

void gfileClose(GFILE *f) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return;
	if (f->vmt->close)
		f->vmt->close(f);
	f->flags = 0;
}

size_t gfileRead(GFILE *f, void *buf, size_t len) {
	size_t	res;

	if (!f || (f->flags & (GFILEFLG_OPEN|GFILEFLG_READ)) != (GFILEFLG_OPEN|GFILEFLG_READ))
		return 0;
	if (!f->vmt->read)
		return 0;
	if ((res = f->vmt->read(f, buf, len)) <= 0)
		return 0;
	f->pos += res;
	return res;
}

size_t gfileWrite(GFILE *f, const void *buf, size_t len) {
	size_t	res;

	if (!f || (f->flags & (GFILEFLG_OPEN|GFILEFLG_WRITE)) != (GFILEFLG_OPEN|GFILEFLG_WRITE))
		return 0;
	if (!f->vmt->write)
		return 0;
	if ((res = f->vmt->write(f, buf, len)) <= 0)
		return 0;
	f->pos += res;
	return res;
}

long int gfileGetPos(GFILE *f) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return 0;
	return f->pos;
}

bool_t gfileSetPos(GFILE *f, long int pos) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return FALSE;
	if (!f->vmt->setpos || !f->vmt->setpos(f, pos))
		return FALSE;
	f->pos = pos;
	return TRUE;
}

long int gfileGetSize(GFILE *f) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return 0;
	if (!f->vmt->getsize)
		return 0;
	return f->vmt->getsize(f);
}

bool_t gfileEOF(GFILE *f) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return TRUE;
	if (!f->vmt->eof)
		return FALSE;
	return f->vmt->eof(f);
}

/********************************************************
 * String VMT routines
 ********************************************************/
#if GFILE_NEED_STRINGS && (GFILE_NEED_PRINTG || GFILE_NEED_SCANG)
	#include <string.h>

	// Special String VMT
	static int StringRead(GFILE *f, void *buf, int size) {
		// size must be 1 for a complete read
		if (!((char *)f->obj)[f->pos])
			return 0;
		((char *)buf)[0] = ((char *)f->obj)[f->pos];
		return 1;
	}
	static int StringWrite(GFILE *f, const void *buf, int size) {
		// size must be 1 for a complete write
		((char *)f->obj)[f->pos] = ((char *)buf)[0];
		return 1;
	}
	static const GFILEVMT StringVMT = {
		0,								// next
		0,								// flags
		'_',							// prefix
		0, 0, 0, 0,
		0, 0, StringRead, StringWrite,
		0, 0, 0,
	};
#endif

/********************************************************
 * printg routines
 ********************************************************/
#if GFILE_NEED_PRINTG
	#include <stdarg.h>

	#define MAX_FILLER		11
	#define FLOAT_PRECISION 100000

	int fnprintg(GFILE *f, int maxlen, const char *fmt, ...) {
		int		res;
		va_list	ap;

		va_start(ap, fmt);
		res = vfnprintg(f, maxlen, fmt, ap);
		va_end(ap);
		return res;
	}

	static char *ltoa_wd(char *p, long num, unsigned radix, long divisor) {
		int		i;
		char *	q;

		if (!divisor) divisor = num;

		q = p + MAX_FILLER;
		do {
			i = (int)(num % radix);
			i += '0';
			if (i > '9')
			  i += 'A' - '0' - 10;
			*--q = i;
			num /= radix;
		} while ((divisor /= radix) != 0);

		i = (int)(p + MAX_FILLER - q);
		do {
			*p++ = *q++;
		} while (--i);

		return p;
	}

	int vfnprintg(GFILE *f, int maxlen, const char *fmt, va_list arg) {
		int		ret;
		char	*p, *s, c, filler;
		int		i, precision, width;
		bool_t	is_long, left_align;
		long	l;
		#if GFILE_ALLOW_FLOATS
			float	f;
			char	tmpbuf[2*MAX_FILLER + 1];
		#else
			char	tmpbuf[MAX_FILLER + 1];
		#endif

		ret = 0;
		if (maxlen < 0)
			return 0;
		if (!maxlen)
			maxlen = -1;

		while (*fmt) {
			if (*fmt != '%') {
				gfileWrite(f, fmt, 1);
				ret++; if (--maxlen) return ret;
				fmt++;
				continue;
			}
			fmt++;

			p = s = tmpbuf;
			left_align = FALSE;
			filler = ' ';
			width = 0;
			precision = 0;

			if (*fmt == '-') {
				fmt++;
				left_align = TRUE;
			}
			if (*fmt == '.') {
				fmt++;
				filler = '0';
			}

			while (1) {
				c = *fmt++;
				if (c >= '0' && c <= '9')
					c -= '0';
				else if (c == '*')
					c = va_arg(ap, int);
				else
					break;
				width = width * 10 + c;
			}
			if (c == '.') {
				while (1) {
					c = *fmt++;
					if (c >= '0' && c <= '9')
						c -= '0';
					else if (c == '*')
						c = va_arg(ap, int);
					else
						break;
					precision = precision * 10 + c;
				}
			}
			/* Long modifier.*/
			if (c == 'l' || c == 'L') {
				is_long = TRUE;
				if (*fmt)
					c = *fmt++;
			}
			else
				is_long = (c >= 'A') && (c <= 'Z');

			/* Command decoding.*/
			switch (c) {
			case 0:
				return ret;
			case 'c':
				filler = ' ';
				*p++ = va_arg(ap, int);
				break;
			case 's':
				filler = ' ';
				if ((s = va_arg(ap, char *)) == 0)
					s = "(null)";
				if (precision == 0)
					precision = 32767;
				for (p = s; *p && (--precision >= 0); p++);
				break;
			case 'D':
			case 'd':
				if (is_long)
					l = va_arg(ap, long);
				else
					l = va_arg(ap, int);
				if (l < 0) {
					*p++ = '-';
					l = -l;
				}
				p = ltoa_wd(p, l, 10, 0);
				break;
			#if GFILE_ALLOW_FLOATS
				case 'f':
					f = (float) va_arg(ap, double);
					if (f < 0) {
						*p++ = '-';
						f = -f;
					}
					l = f;
					p = ltoa_wd(p, l, 10, 0);
					*p++ = '.';
					l = (f - l) * FLOAT_PRECISION;
					p = ltoa_wd(p, l, 10, FLOAT_PRECISION / 10);
					break;
			#endif
			case 'X':
			case 'x':
				c = 16;
				goto unsigned_common;
			case 'U':
			case 'u':
				c = 10;
				goto unsigned_common;
			case 'O':
			case 'o':
				c = 8;
			unsigned_common:
				if (is_long)
					l = va_arg(ap, long);
				else
					l = va_arg(ap, int);
				p = ltoa_wd(p, l, c, 0);
				break;
			default:
				*p++ = c;
				break;
			}

			i = (int)(p - s);
			if ((width -= i) < 0)
				width = 0;
			if (left_align == FALSE)
				width = -width;
			if (width < 0) {
				if (*s == '-' && filler == '0') {
					gfileWrite(f, s++, 1);
					ret++; if (--maxlen) return ret;
					i--;
				}
				do {
					gfileWrite(f, &filler, 1);
					ret++; if (--maxlen) return ret;
				} while (++width != 0);
			}
			while (--i >= 0) {
				gfileWrite(f, s++, 1);
				ret++; if (--maxlen) return ret;
			}
			while (width) {
				gfileWrite(f, &filler, 1);
				ret++; if (--maxlen) return ret;
				width--;
			}
		}
		return ret;
	}

	#if GFILE_NEED_STRINGS
		int snprintg(char *buf, int maxlen, const char *fmt, ...) {
			int		res;
			GFILE	f;
			va_list	ap;

			if (maxlen <= 1) {
				if (maxlen == 1)
					*buf = 0;
				return 0;
			}
			f.flags = GFILEFLG_OPEN|GFILEFLG_WRITE;
			f.vmt = &StringVMT;
			f.pos = 0;
			f.obj = buf;
			va_start(ap, fmt);
			res = vfnprintg(&f, maxlen-1, fmt, ap);
			va_end(ap);
			buf[res] = 0;
			return res;
		}
		int vsnprintg(char *buf, int maxlen, const char *fmt, va_list arg) {
			int		res;
			GFILE	f;

			if (maxlen <= 1) {
				if (maxlen == 1)
					*buf = 0;
				return 0;
			}
			f.flags = GFILEFLG_OPEN|GFILEFLG_WRITE;
			f.vmt = &StringVMT;
			f.pos = 0;
			f.obj = buf;
			res = vfnprintg(&f, maxlen-1, fmt, arg);
			buf[res] = 0;
			return res;
		}
	#endif
#endif

/********************************************************
 * scang routines
 ********************************************************/
#if GFILE_NEED_SCANG
	int fscang(GFILE *f, const char *fmt, ...) {
		int		res;
		va_list	ap;

		va_start(ap, fmt);
		res = vfscang(f, fmt, ap);
		va_end(ap);
		return res;
	}

	int vfscang(GFILE *f, const char *fmt, va_list arg) {
		int		res, width, size, base;
		char	c;
		bool_t	assign;
		void	*p;

		for(res = 0; *fmt; fmt++) {
			switch(*fmt) {
			case ' ': case '\t': case '\r': case '\n': case '\v': case '\f':
				break;

			case '%':
				fmt++;
				assign = TRUE;
				width = 0;
				size = 1;

				if (*fmt == '*') {
					fmt++;
					assign = FALSE;
				}
				while(*fmt >= '0' && *fmt <= '9')
					width = width * 10 + (*fmt++ - '0');
				if (*fmt == 'h') {
					fmt++;
					size = 0;
				} else if (*fmt == 'l') {
					fmt++;
					size = 2;
				} else if (*fmt == 'L') {
					fmt++;
					size = 3;
				}
				switch(*fmt) {
				case 0:
					return res;
				case '%':
					goto matchchar;
				case 'c':
					if (!width) {
						while(1) {
							if (!gfileRead(f, &c, 1))			return res;
							switch(c) {
							case ' ': case '\t': case '\r':
							case '\n': case '\v': case '\f':	continue;
							}
							break;
						}
						width = 1;
					} else {
						if (!gfileRead(f, &c, 1)) 				return res;
					}
					if (assign) {
						p = va_arg(ap, char *);
						res++;
						*((char *)p)++ = c;
					}
					while(--width) {
						if (!gfileRead(f, &c, 1)) 			return res;
						if (assign) *((char *)p)++ = c;
					}
					break;
				case 's':
					while(1) {
						if (!gfileRead(f, &c, 1))			return res;
						switch(c) {
						case ' ': case '\t': case '\r':
						case '\n': case '\v': case '\f':	continue;
						}
						break;
					}
					if (assign) {
						p = va_arg(ap, char *);
						res++;
						*((char *)p)++ = c;
					}
					if (width) {
						while(--width) {
							if (!gfileRead(f, &c, 1)) {
								if (assign) *((char *)p) = 0;
								return res;
							}
							if (assign) *((char *)p)++ = c;
						}
					} else {
						while(1) {
							if (!gfileRead(f, &c, 1)) {
								if (assign) *((char *)p) = 0;
								return res;
							}
							switch(c) {
							case ' ': case '\t': case '\r':
							case '\n': case '\v': case '\f':	break;
							default:
								if (assign) *((char *)p)++ = c;
								continue;
							}
							break;
						}
						//ungetch(c);
					}
					if (assign) *((char *)p) = 0;
					break;
				case 'd':
				case 'i':
				case 'o':
				case 'u':
				case 'x':
				case 'b':
					/*
					while (isspace (*buf))
					    buf++;
					if (*s == 'd' || *s == 'u')
					    base = 10;
					else if (*s == 'x')
					    base = 16;
					else if (*s == 'o')
					    base = 8;
					else if (*s == 'b')
					    base = 2;
					if (!width) {
					    if (isspace (*(s + 1)) || *(s + 1) == 0)
						width = strcspn (buf, ISSPACE);
					    else
						width = strchr (buf, *(s + 1)) - buf;
					}
					strncpy (tmp, buf, width);
					tmp[width] = '\0';
					buf += width;
					if (!noassign)
					    atob (va_arg (ap, u_int32_t *), tmp, base);
				    }
				    if (!noassign)
					count++;
					*/

				#if GFILE_ALLOW_FLOATS
					case 'e': case 'f': case 'g':
				#endif
				default:
					return res;
				}

				break;

			default:
			matchchar:
				while(1) {
					if (!gfileRead(f, &c, 1))			return res;
					switch(c) {
					case ' ': case '\t': case '\r':
					case '\n': case '\v': case '\f':	continue;
					}
					break;
				}
				if (c != *fmt) 							return res;
				break;
			}
		}
		return res;
	}

	#if GFILE_NEED_STRINGS
		int sscang(const char *buf, const char *fmt, ...) {
			int		res;
			GFILE	f;
			va_list	ap;

			f.flags = GFILEFLG_OPEN|GFILEFLG_READ;
			f.vmt = &StringVMT;
			f.pos = 0;
			f.obj = buf;
			va_start(ap, fmt);
			res = vfscang(&f, fmt, ap);
			va_end(ap);
			return res;
		}

		int vsscang(const char *buf, const char *fmt, va_list arg) {
			int		res;
			GFILE	f;

			f.flags = GFILEFLG_OPEN|GFILEFLG_READ;
			f.vmt = &StringVMT;
			f.pos = 0;
			f.obj = buf;
			res = vfscang(&f, fmt, arg);
			return res;
		}
	#endif
#endif

/********************************************************
 * stdio emulation routines
 ********************************************************/
#if GFILE_NEED_STDIO
	size_t gstdioRead(void * ptr, size_t size, size_t count, FILE *f) {
		return gfileRead(f, ptr, size*count)/size;
	}
	size_t gstdioWrite(const void * ptr, size_t size, size_t count, FILE *f) {
		return gfileWrite(f, ptr, size*count)/size;
	}
	int gstdioSeek(FILE *f, size_t offset, int origin) {
		switch(origin) {
		case SEEK_SET:
			break;
		case SEEK_CUR:
			offset += f->pos;
			break;
		case SEEK_END:
			offset += gfileGetSize(f);
			break;
		default:
			return -1;
		}
		return gfileSetPos(f, offset) ? 0 : -1;
	}
	int gstdioGetpos(FILE *f, long int *pos) {
		if (!(f->flags & GFILEFLG_OPEN))
			return -1;
		*pos = f->pos;
		return 0;
	}
#endif

#endif /* GFX_USE_GFILE */
