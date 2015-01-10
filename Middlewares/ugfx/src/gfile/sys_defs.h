/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gfile/sys_defs.h
 * @brief   GFILE - File IO Routines header file.
 *
 * @addtogroup GFILE
 *
 * @brief	Module which contains Operating system independent FILEIO
 *
 * @{
 */

#ifndef _GFILE_H
#define _GFILE_H

#include "gfx.h"

#if GFX_USE_GFILE || defined(__DOXYGEN__)

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

/**
 * @brief	A file pointer
 */

#ifndef GFILE_IMPLEMENTATION
	typedef void GFILE;
#else
	typedef struct GFILE GFILE;
#endif

extern GFILE *gfileStdIn;
extern GFILE *gfileStdErr;
extern GFILE *gfileStdOut;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief					Check if file exists
	 * 
	 * @param[in] fname			The file name
	 * 
	 * @return					TRUE if file exists, FALSE otherwise
	 * 
	 * @api
	 */
	bool_t		gfileExists(const char *fname);

	/**
	 * @brief					Delete file
	 * 
	 * @param[in] fname			The file name
	 * 
	 * @return					TRUE on success, FALSE otherwise
	 * 
	 * @api
	 */
	bool_t		gfileDelete(const char *fname);

	/**
	 * @brief					Get the size of a file
	 * @note					Please use @p gfileGetSize() if the file is not opened
	 * 
	 * @param[in] fname			The file name
	 * 
	 * @return					File size on success, -1 on error
	 * 
	 * @api
	 */
	long int	gfileGetFilesize(const char *fname);

	/**
	 * @brief					Rename file
	 *
	 * @param[in] oldname		The current file name
	 * @param[in] newname		The new name of the file
	 *
	 * @return					TRUE on success, FALSE otherwise
	 *
	 * @api
	 */
	bool_t		gfileRename(const char *oldname, const char *newname);

	/**
	 * @brief					Open file
	 * @details					A file must be opened before it can be accessed
	 * @details					ToDo (document possible modes)
	 * @details					The resulting GFILE will be used for all functions that access the file.
	 *
	 * @param[in] fname			The file name
	 * @param[in] mode			The mode
	 *
	 * @return					Valid GFILE on success, 0 otherwise
	 *
	 * @api
	 */	
	GFILE *		gfileOpen(const char *fname, const char *mode);

	/**
	 * @brief					Close file
	 * @details					Closes a file after is has been opened using @p gfileOpen()
	 *
	 * @param[in] f				The file
	 *
	 * @api
	 */
	void		gfileClose(GFILE *f);

	/**
	 * @brief					Read from file
	 * @details					Reads a given amount of bytes from the file
	 * @details					The read/write cursor will not be reset when calling this function
	 *
	 * @param[in] f				The file
	 * @param[out] buf			The buffer in which to save the content that has been read from the file
	 * @param[in] len			Amount of bytes to read
	 *
	 * @return					Amount of bytes read
	 *
	 * @api
	 */
	size_t		gfileRead(GFILE *f, void *buf, size_t len);

	/**
	 * @brief					Write to file
	 * @details					Write a given amount of bytes to the file
	 * @details					The read/write cursor will not be reset when calling this function
	 *
	 * @param[in] f				The file
	 * @param[in] buf			The buffer which contains the content that will be written to the file
	 * @param[in] len			Amount of bytes to write
	 *
	 * @return					Amount of bytes written
	 *
	 * @api
	 */
	size_t		gfileWrite(GFILE *f, const void *buf, size_t len);

	/**
	 * @brief					Get the current position of the read/write cursor
	 *
	 * @param[in] f				The file
	 *
	 * @return					The current position in the file
	 *
	 * @api
	 */
	long int	gfileGetPos(GFILE *f);

	/**
	 * @brief					Set the position of the read/write cursor
	 *
	 * @param[in] f				The file
	 * @param[in] pos			The position to which the cursor will be set
	 *
	 * @return					TRUE on success, FALSE otherwise
	 *
	 * @api
	 */
	bool_t		gfileSetPos(GFILE *f, long int pos);

	/**
	 * @brief					Get the size of file
	 * @note					Please use @p gfileGetFilesize() if the file is not opened
	 *
	 * @param[in] f				The file
	 *
	 * @return					The size of the file
	 *
	 * @api
	 */
	long int	gfileGetSize(GFILE *f);

	/**
	 * @brief					Check for EOF
	 * @details					Checks if the cursor is at the end of the file
	 *
	 * @param[in] f				The file
	 *
	 * @return					TRUE if EOF, FALSE otherwise
	 *
	 * @api
	 */
	bool_t		gfileEOF(GFILE *f);

	#if GFILE_NEED_CHIBIOSFS && GFX_USE_OS_CHIBIOS
		GFILE *		gfileOpenBaseFileStream(void *BaseFileStreamPtr, const char *mode);
	#endif
	#if GFILE_NEED_MEMFS
		GFILE *		gfileOpenMemory(void *memptr, const char *mode);
	#endif

	#if GFILE_NEED_PRINTG
		int vfnprintg(GFILE *f, int maxlen, const char *fmt, va_list arg);
		int fnprintg(GFILE *f, int maxlen, const char *fmt, ...);
		#define vfprintg(f,m,a)			vfnprintg(f,0,m,a)
		#define fprintg(f,m,...)		fnprintg(f,0,m,...)
		#define vprintg(m,a)			vfnprintg(gfileStdOut,0,m,a)
		#define printg(m,...)			fnprintg(gfileStdOut,0,m,...)

		#if GFILE_NEED_STRINGS
			int vsnprintg(char *buf, int maxlen, const char *fmt, va_list arg);
			int snprintg(char *buf, int maxlen, const char *fmt, ...);
			#define vsprintg(s,m,a)		vsnprintg(s,0,m,a)
			#define sprintg(s,m,...)	snprintg(s,0,m,...)
		#endif
	#endif

	#if GFILE_NEED_SCANG
		int vfscang(GFILE *f, const char *fmt, va_list arg);
		int fscang(GFILE *f, const char *fmt, ...);
		#define vscang(f,a)			vfscang(gfileStdIn,f,a)
		#define scang(f,...)		fscang(gfileStdIn,f,...)

		#if GFILE_NEED_STRINGS
			int vsscang(const char *buf, const char *fmt, va_list arg);
			int sscang(const char *buf, const char *fmt, ...);
		#endif
	#endif

	#if GFILE_NEED_STDIO && !defined(GFILE_IMPLEMENTATION)
		#define stdin					gfileStdIn
		#define stdout					gfileStdOut
		#define stderr					gfileStdErr
		#define FILENAME_MAX			256						// Use a relatively small number for an embedded platform
		#define L_tmpnam				FILENAME_MAX
		#define FOPEN_MAX				GFILE_MAX_GFILES
		#define TMP_MAX					GFILE_MAX_GFILES
		#define P_tmpdir				"/tmp/"
		#define FILE					GFILE
		#define fopen(n,m)				gfileOpen(n,m)
		#define fclose(f)				gfileClose(f)
		size_t gstdioRead(void * ptr, size_t size, size_t count, FILE *f);
		size_t gstdioWrite(const void * ptr, size_t size, size_t count, FILE *f);
		#define fread(p,sz,cnt,f)		gstdioRead(p,sz,cnt,f)
		#define fwrite(p,sz,cnt,f)		gstdioWrite(p,sz,cnt,f)
		int gstdioSeek(FILE *f, size_t offset, int origin);
		#define fseek(f,ofs,org)		gstdioSeek(f,ofs,org)
			#define SEEK_SET	0
			#define SEEK_CUR	1
			#define SEEK_END	2
		#define remove(n)				(!gfileDelete(n))
		#define rename(o,n)				(!gfileRename(o,n))
		#define fflush(f)				(0)
		#define ftell(f)				gfileGetPos(f)
		#define fpos_t					long int
		int gstdioGetpos(FILE *f, long int *pos);
		#define fgetpos(f,pos)			gstdioGetpos(f,pos)
		#define fsetpos(f, pos)			(!gfileSetPos(f, *pos))
		#define rewind(f)				gfileSetPos(f, 0);
		#define feof(f)					gfileEOF(f)

		#define vfprintf(f,m,a)			vfnprintg(f,0,m,a)
		#define fprintf(f,m,...)		fnprintg(f,0,m,...)
		#define vprintf(m,a)			vfnprintg(gfileStdOut,0,m,a)
		#define printf(m,...)			fnprintg(gfileStdOut,0,m,...)
		#define vsnprintf(s,n,m,a)		vsnprintg(s,n,m,a)
		#define snprintf(s,n,m,...)		snprintg(s,n,m,...)
		#define vsprintf(s,m,a)			vsnprintg(s,0,m,a)
		#define sprintf(s,m,...)		snprintg(s,0,m,...)
		//TODO
		//void clearerr ( FILE * stream );
		//int ferror ( FILE * stream );
		//FILE * tmpfile ( void );		// Auto-deleting
		//char * tmpnam ( char * str );
		//char * mktemp (char *template);
		//FILE * freopen ( const char * filename, const char * mode, FILE * stream );
		//setbuf
		//setvbuf
		//fflush
		//fgetc
		//fgets
		//fputc
		//fputs
		//getc
		//getchar
		//puts
		//ungetc
		//void perror (const char * str);
	#endif

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_GFILE */

#endif /* _GFILE_H */
/** @} */

