// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "gfx.h"

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"




int	mb_used = 6;


void
I_Tactile
( int	on,
  int	off,
  int	total )
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t	emptycmd;
ticcmd_t*	I_BaseTiccmd(void)
{
    return &emptycmd;
}


int  I_GetHeapSize (void)
{
    return mb_used*1024*1024;
}

byte* I_ZoneBase (int*	size)
{
    *size = mb_used*1024*1024;
    return gfxAlloc (*size);
}



//
// I_GetTime
// returns time in 1/70th second tics
//
int  I_GetTime (void)
{
	systemticks_t	tdiv;

	tdiv = gfxMillisecondsToTicks(1000*256/TICRATE);
	return (gfxSystemTicks()<<8)/tdiv;
}



//
// I_Init
//
void I_Init (void)
{
    I_InitSound();
    //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
    D_QuitNetGame ();
    I_ShutdownSound();
    I_ShutdownMusic();
    M_SaveDefaults ();
    I_ShutdownGraphics();
    gfxExit();
}

void I_WaitVBL(int count)
{
    gfxSleepMilliseconds(1000/TICRATE);
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

byte*	I_Malloc(int length)
{
    byte*	mem;
        
    mem = gfxAlloc (length);
    memset (mem,0,length);
    return mem;
}


//
// I_Error
//
extern boolean demorecording;

void I_Error (char *error, ...)
{
#if 0
    va_list	argptr;

    // Message first.
    va_start (argptr,error);
    fprintf (stderr, "Error: ");
    vfprintf (stderr,error,argptr);
    fprintf (stderr, "\n");
    va_end (argptr);

    fflush( stderr );
#endif

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownGraphics();
    
    gfxHalt(error);
}

#include "doom1_wad.h"

#define F1NAME		"doom1.wad"
#define	F1SIZE		4196020
#define F1HANDLE	313			// Some random number > 2
static size_t		f1pos;

int I_HaveFile(char *fname) {
	return !strcmp(fname, F1NAME);
}

int I_FileSize(int handle) {
	return handle == F1HANDLE ? F1SIZE : 0;
}

int I_FileRead(int handle, char *buf, int len) {
	if (handle != F1HANDLE || len <= 0) return 0;
	if (f1pos + len > F1SIZE)
		len = F1SIZE - f1pos;
	memcpy(buf, doom1_wad+f1pos, len);
	return len;
}
void I_FilePos(int handle, int pos) {
	if (handle != F1HANDLE) return;
	if (pos > F1SIZE)
		pos = F1SIZE;
	f1pos = pos;
}
int I_FileOpenRead(char *fname) {
	return strcmp(fname, F1NAME) ? -1 : F1HANDLE;
}
int I_FileCreate(char *fname) {
	return -1;
}
int I_FileWrite(int handle, char *source, int length) {
	return 0;
}
void I_FileClose(int handle) {
}
void *I_Realloc(void *p, int nsize) {
	return gfxRealloc(p, 0 /* Oops - we don't know this */, nsize);
}
void I_Exit(int code) {
	gfxExit();
}
void I_printf(const char *fmt, ...) {
}
void I_DBGprintf(const char *fmt, ...) {
}

#include <stdarg.h>

#define MAX_FILLER 11
#define FLOAT_PRECISION 100000

static char *ltoa_wd(char *p, long num, unsigned radix, long divisor) {
	int i;
	char *q;

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

#if USE_FLOAT
	static char *ftoa(char *p, double num) {
		long l;
		unsigned long precision = FLOAT_PRECISION;

		l = num;
		p = ltoa_wd(p, l, 10, 0);
		*p++ = '.';
		l = (num - l) * precision;
		return ltoa_wd(p, l, 10, precision / 10);
	}
#endif

void I_sprintf(char *buf, const char *fmt, ...) {
	va_list ap;
	char *p, *s, c, filler;
	int i, precision, width;
	bool_t is_long, left_align;
	long l;
	#if USE_FLOAT
		float f;
		char tmpbuf[2*MAX_FILLER + 1];
	#else
		char tmpbuf[MAX_FILLER + 1];
	#endif

	va_start(ap, fmt);
	while (TRUE) {
		c = *fmt++;
		if (c == 0) {
			va_end(ap);
			*buf = 0;
			return;
		}
		if (c != '%') {
			*buf++ = c;
			continue;
		}

		p = tmpbuf;
		s = tmpbuf;
		left_align = FALSE;
		if (*fmt == '-') {
			fmt++;
			left_align = TRUE;
		}
		filler = ' ';
		if (*fmt == '.') {
			fmt++;
			filler = '0';
		}
		width = 0;

		while (TRUE) {
			c = *fmt++;
			if (c >= '0' && c <= '9')
				c -= '0';
			else if (c == '*')
				c = va_arg(ap, int);
			else
				break;
			width = width * 10 + c;
		}
		precision = 0;
		if (c == '.') {
			while (TRUE) {
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
		#if USE_FLOAT
			case 'f':
				f = (float) va_arg(ap, double);
				if (f < 0) {
					*p++ = '-';
					f = -f;
				}
				p = ftoa(p, f);
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
				*buf++ = *s++;
				i--;
			}
			do {
				*buf++ = filler;
			} while (++width != 0);
		}
		while (--i >= 0)
			*buf++ = *s++;
		while (width) {
			*buf++ = filler;
			width--;
		}
	}
}

