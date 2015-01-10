/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gmisc/sys_defs.h
 * @brief   GMISC - Miscellaneous Routines header file.
 *
 * @addtogroup GMISC
 *
 * @brief	Module which contains different features such as array conversions
 *
 * @{
 */

#ifndef _GMISC_H
#define _GMISC_H

#include "gfx.h"

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

/**
 * @brief	Sample data formats
 * @note	These are defined regardless of whether you use the GMISC module
 * 			or not as they are used in lots of places.
 */
typedef enum ArrayDataFormat_e {
	ARRAY_DATA_4BITUNSIGNED = 4,	ARRAY_DATA_4BITSIGNED = 5,
	ARRAY_DATA_8BITUNSIGNED = 8,	ARRAY_DATA_8BITSIGNED = 9,
	ARRAY_DATA_10BITUNSIGNED = 10,	ARRAY_DATA_10BITSIGNED = 11,
	ARRAY_DATA_12BITUNSIGNED = 12,	ARRAY_DATA_12BITSIGNED = 13,
	ARRAY_DATA_14BITUNSIGNED = 14,	ARRAY_DATA_14BITSIGNED = 15,
	ARRAY_DATA_16BITUNSIGNED = 16,	ARRAY_DATA_16BITSIGNED = 17,
	} ArrayDataFormat;

/**
 * @brief	Is the sample data format a "signed" data format?
 */
#define gfxSampleFormatIsSigned(fmt)	((fmt) & 1)

/**
 * @brief	How many bits are in the sample data format
 */
#define gfxSampleFormatBits(fmt)	((fmt) & ~1)

/**
 * @brief   The type for a fixed point type.
 * @details	The top 16 bits are the integer component, the bottom 16 bits are the real component.
 */
typedef int32_t	fixed;

/**
 * @brief   Macros to convert to and from a fixed point.
 * @{
 */
#define FIXED(x)		((fixed)(x)<<16)			/* @< integer to fixed */
#define NONFIXED(x)		((x)>>16)					/* @< fixed to integer */
#define FIXED0_5		32768						/* @< 0.5 as a fixed (used for rounding) */
#define FP2FIXED(x)		((fixed)((x)*65536.0))		/* @< floating point to fixed */
#define FIXED2FP(x)		((double)(x)/65536.0)		/* @< fixed to floating point */
/* @} */

/**
 * @brief   The famous number pi
 */
#define PI	3.1415926535897932384626433832795028841971693993751

/**
 * @brief   pi as a fixed point
 */
#define FIXED_PI	FP2FIXED(PI)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if GFX_USE_GMISC || defined(__DOXYGEN__)

#ifdef __cplusplus
extern "C" {
#endif

#if GMISC_NEED_ARRAYOPS || defined(__DOXYGEN__)
	/**
	 * @brief				Convert from one array format to another array format.
	 *
	 * @param[in] srcfmt		The format of the source array
	 * @param[in] src			The source array
	 * @param[in] dstfmt		The format of the destination array
	 * @param[in] dst			The dstination array
	 * @param[in] cnt			The number of array elements to convert
	 *
	 * @note				Assumes the destination buffer is large enough for the resultant data.
	 * @note				This routine is optimised to perform as fast as possible.
	 * @note				No type checking is performed on the source format. It is assumed to
	 * 						have only valid values eg. ARRAY_DATA_4BITSIGNED will have values
	 * 							0000 -> 0111 for positive numbers and 1111 -> 1000 for negative numbers
	 * 							Bits 5 -> 8 in the storage byte are treated in an undefined manner.
	 * @note				If srcfmt or dstfmt is an unknown format, this routine does nothing
	 * 						with no warning that something is wrong
	 *
	 * @api
	 */
	void gmiscArrayConvert(ArrayDataFormat srcfmt, void *src, ArrayDataFormat dstfmt, void *dst, size_t cnt);

	#if 0
		void gmiscArrayTranslate(ArrayDataFormat fmt, void *src, void *dst, size_t cnt, int trans);

		void gmiscArrayMultiply(ArrayDataFormat fmt, void *src, void *dst, size_t cnt, int mult);

		void gmiscArrayDivide(ArrayDataFormat fmt, void *src, void *dst, size_t cnt, int mdiv);

		void gmiscArrayMultDiv(ArrayDataFormat fmt, void *src, void *dst, size_t cnt, int mult, int div);

		void gmiscArrayAdd(ArrayDataFormat fmt, void *src1, void *src2, void *dst, size_t cnt);

		void gmiscArrayAddNoOverflow(ArrayDataFormat fmt, void *src1, void *src2, void *dst, size_t cnt);
	#endif
#endif

#if GMISC_NEED_FASTTRIG || defined(__DOXYGEN__)
		extern const double sintabledouble[];

		/**
		 * @brief	Fast Table Based Trig functions
		 * @return	A double in the range -1.0 .. 0.0 .. 1.0
		 *
		 * @param[in] degrees	The angle in degrees (not radians)
		 *
		 * @note	These functions use degrees rather than radians to describe the angle.
		 *
		 * @api
		 * @{
		 */
		double fsin(int degrees);
		double fcos(int degrees);
		/** @}
		 *
		 * @brief	Fast Table Based Trig functions
		 * @return	A double in the range -1.0 .. 0.0 .. 1.0
		 *
		 * @param[in] degrees	The angle in degrees 0 .. 359
		 *
		 * @note	These functions use degrees rather than radians to describe the angle.
		 * @note	These functions are super fast but require the parameter to be in range.
		 * 			Use the lowercase functions if the parameter may not be in range or if a
		 * 			required trig function is not supported in this form.
		 *
		 * @api
		 * @{
		 */
		#define FSIN(degrees) 	sintabledouble[degrees];
		/** @} */
#endif

#if GMISC_NEED_FIXEDTRIG || defined(__DOXYGEN__)
		extern const fixed sintablefixed[];

		/**
		 * @brief	Fast Table Based Trig functions
		 * @return	A fixed point in the range -1.0 .. 0.0 .. 1.0
		 *
		 * @param[in] degrees	The angle in degrees (not radians)
		 *
		 * @note	These functions use degrees rather than radians to describe the angle.
		 *
		 * @api
		 * @{
		 */
		fixed ffsin(int degrees);
		fixed ffcos(int degrees);
		/** @}
		 *
		 * @brief	Fast Table Based Trig functions
		 * @return	A fixed point in the range -1.0 .. 0.0 .. 1.0
		 *
		 * @param[in] degrees	The angle in degrees 0 .. 359
		 *
		 * @note	These functions use degrees rather than radians to describe the angle.
		 * @note	These functions are super fast but require the parameter to be in range.
		 * 			Use the lowercase functions if the parameter may not be in range or if a
		 * 			required trig function is not supported in this form.
		 *
		 * @api
		 * @{
		 */
		#define FFSIN(degrees) 	sintablefixed[degrees];
		/** @} */
#endif

#if GMISC_NEED_INVSQRT
		/**
		 * @brief	Fast inverse square root function (x^-1/2)
		 * @return	The approximate inverse square root
		 *
		 * @param[in] n	The number to find the inverse square root of
		 *
		 * @note	This function generates an approximate result. Higher accuracy (at the expense
		 * 			of speed) can be obtained by modifying the source code (the necessary line
		 * 			is already there - just commented out).
		 * @note	This function relies on the internal machine format of a float and a long.
		 * 			If your machine architecture is very unusual this function may not work.
		 *
		 * @api
		 */
		float invsqrt(float n);
#endif
#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_MISC */

#endif /* _GMISC_H */
/** @} */

