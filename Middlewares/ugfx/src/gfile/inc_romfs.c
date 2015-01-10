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
 * The ROM file-system VMT
 ********************************************************/

#include <string.h>

// What directory file formats do we understand
#define ROMFS_DIR_VER_MAX			0x0000

// Compression Formats
#define ROMFS_CMP_UNCOMPRESSED		0

typedef struct ROMFS_DIRENTRY {
	uint16_t						ver;			// Directory Entry Version
	uint16_t						cmp;			// Compression format
	const struct ROMFS_DIRENTRY *	next;			// The next entry
	const char *					name;			// The file name
	long int						size;			// The file size
	const char *					file;			// The file data
} ROMFS_DIRENTRY;

#define ROMFS_DIRENTRY_HEAD		0

#include "romfs_files.h"

static const ROMFS_DIRENTRY const *FsROMHead = ROMFS_DIRENTRY_HEAD;

static bool_t ROMExists(const char *fname);
static long int	ROMFilesize(const char *fname);
static bool_t ROMOpen(GFILE *f, const char *fname);
static void ROMClose(GFILE *f);
static int ROMRead(GFILE *f, void *buf, int size);
static bool_t ROMSetpos(GFILE *f, long int pos);
static long int ROMGetsize(GFILE *f);
static bool_t ROMEof(GFILE *f);

static const GFILEVMT FsROMVMT = {
	GFILE_CHAINHEAD,									// next
	GFSFLG_CASESENSITIVE|GFSFLG_SEEKABLE|GFSFLG_FAST,	// flags
	'S',												// prefix
	0, ROMExists, ROMFilesize, 0,
	ROMOpen, ROMClose, ROMRead, 0,
	ROMSetpos, ROMGetsize, ROMEof,
};
#undef GFILE_CHAINHEAD
#define GFILE_CHAINHEAD		&FsROMVMT

static const ROMFS_DIRENTRY *ROMFindFile(const char *fname)
{
	const ROMFS_DIRENTRY *p;

	for(p = FsROMHead; p; p = p->next) {
		if (p->ver <= ROMFS_DIR_VER_MAX && p->cmp == ROMFS_CMP_UNCOMPRESSED && !strcmp(p->name, fname))
			break;
	}
	return p;
}

static bool_t ROMExists(const char *fname)
{
	return ROMFindFile(fname) != 0;
}

static long int	ROMFilesize(const char *fname)
{
	const ROMFS_DIRENTRY *p;

	if (!(p = ROMFindFile(fname))) return -1;
	return p->size;
}

static bool_t ROMOpen(GFILE *f, const char *fname)
{
	const ROMFS_DIRENTRY *p;

	if (!(p = ROMFindFile(fname))) return FALSE;
	f->obj = (void *)p;
	return TRUE;
}

static void ROMClose(GFILE *f)
{
	(void)f;
}

static int ROMRead(GFILE *f, void *buf, int size)
{
	const ROMFS_DIRENTRY *p;

	p = (const ROMFS_DIRENTRY *)f->obj;
	if (p->size - f->pos < size)
		size = p->size - f->pos;
	if (size <= 0)	return 0;
	memcpy(buf, p->file+f->pos, size);
	return size;
}

static bool_t ROMSetpos(GFILE *f, long int pos)
{
	return pos <= ((const ROMFS_DIRENTRY *)f->obj)->size;
}

static long int ROMGetsize(GFILE *f)
{
	return ((const ROMFS_DIRENTRY *)f->obj)->size;
}

static bool_t ROMEof(GFILE *f)
{
	return f->pos >= ((const ROMFS_DIRENTRY *)f->obj)->size;
}
