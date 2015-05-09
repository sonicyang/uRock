/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GDISP && GDISP_NEED_IMAGE && GDISP_NEED_IMAGE_NATIVE

/**
 * How big a pixel array to allocate for blitting
 * Bigger is faster but uses more RAM.
 */
#define BLIT_BUFFER_SIZE	32

#define HEADER_SIZE			8
#define FRAME0POS			(HEADER_SIZE)

/**
 * Helper Routines Needed
 */
void *gdispImageAlloc(gdispImage *img, size_t sz);
void gdispImageFree(gdispImage *img, void *ptr, size_t sz);

typedef struct gdispImagePrivate {
	pixel_t		*frame0cache;
	pixel_t		buf[BLIT_BUFFER_SIZE];
	} gdispImagePrivate;

void gdispImageClose_NATIVE(gdispImage *img) {
	if (img->priv) {
		if (img->priv->frame0cache)
			gdispImageFree(img, (void *)img->priv->frame0cache, img->width * img->height * sizeof(pixel_t));
		gdispImageFree(img, (void *)img->priv, sizeof(gdispImagePrivate));
		img->priv = 0;
	}
}

gdispImageError gdispImageOpen_NATIVE(gdispImage *img) {
	uint8_t		hdr[HEADER_SIZE];

	/* Read the 8 byte header */
	if (gfileRead(img->f, hdr, 8) != 8)
		return GDISP_IMAGE_ERR_BADFORMAT;		// It can't be us

	if (hdr[0] != 'N' || hdr[1] != 'I')
		return GDISP_IMAGE_ERR_BADFORMAT;		// It can't be us

	if (hdr[6] != GDISP_PIXELFORMAT/256 || hdr[7] != (GDISP_PIXELFORMAT & 0xFF))
		return GDISP_IMAGE_ERR_UNSUPPORTED;		// Unsupported pixel format

	/* We know we are a native format image */
	img->flags = 0;
	img->width = (((uint16_t)hdr[2])<<8) | (hdr[3]);
	img->height = (((uint16_t)hdr[4])<<8) | (hdr[5]);
	if (img->width < 1 || img->height < 1)
		return GDISP_IMAGE_ERR_BADDATA;
	if (!(img->priv = (gdispImagePrivate *)gdispImageAlloc(img, sizeof(gdispImagePrivate))))
		return GDISP_IMAGE_ERR_NOMEMORY;
	img->priv->frame0cache = 0;

	img->type = GDISP_IMAGE_TYPE_NATIVE;
	return GDISP_IMAGE_ERR_OK;
}

gdispImageError gdispImageCache_NATIVE(gdispImage *img) {
	size_t		len;

	/* If we are already cached - just return OK */
	if (img->priv->frame0cache)
		return GDISP_IMAGE_ERR_OK;

	/* We need to allocate the cache */
	len = img->width * img->height * sizeof(pixel_t);
	img->priv->frame0cache = (pixel_t *)gdispImageAlloc(img, len);
	if (!img->priv->frame0cache)
		return GDISP_IMAGE_ERR_NOMEMORY;

	/* Read the entire bitmap into cache */
	gfileSetPos(img->f, FRAME0POS);
	if (gfileRead(img->f, img->priv->frame0cache, len) != len)
		return GDISP_IMAGE_ERR_BADDATA;

	return GDISP_IMAGE_ERR_OK;
}

gdispImageError gdispGImageDraw_NATIVE(GDisplay *g, gdispImage *img, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t sx, coord_t sy) {
	coord_t		mx, mcx;
	size_t		pos, len;

	/* Check some reasonableness */
	if (sx >= img->width || sy >= img->height) return GDISP_IMAGE_ERR_OK;
	if (sx + cx > img->width) cx = img->width - sx;
	if (sy + cy > img->height) cy = img->height - sy;

	/* Draw from the image cache - if it exists */
	if (img->priv->frame0cache) {
		gdispGBlitArea(g, x, y, cx, cy, sx, sy, img->width, img->priv->frame0cache);
		return GDISP_IMAGE_ERR_OK;
	}

	/* For this image decoder we cheat and just seek straight to the region we want to display */
	pos = FRAME0POS + (img->width * sy + sx) * sizeof(pixel_t);

	/* Cycle through the lines */
	for(;cy;cy--, y++) {
		/* Move to the start of the line */
		gfileSetPos(img->f, pos);

		/* Draw the line in chunks using BitBlt */
		for(mx = x, mcx = cx; mcx > 0; mcx -= len, mx += len) {
			// Read the data
			len = gfileRead(img->f,
						img->priv->buf,
						mcx > BLIT_BUFFER_SIZE ? (BLIT_BUFFER_SIZE*sizeof(pixel_t)) : (mcx * sizeof(pixel_t)))
					/ sizeof(pixel_t);
			if (!len)
				return GDISP_IMAGE_ERR_BADDATA;

			/* Blit the chunk of data */
			gdispGBlitArea(g, mx, y, len, 1, 0, 0, len, img->priv->buf);
		}

		/* Get the position for the start of the next line */
		pos += img->width*sizeof(pixel_t);
	}

	return GDISP_IMAGE_ERR_OK;
}

delaytime_t gdispImageNext_NATIVE(gdispImage *img) {
	(void) img;

	/* No more frames/pages */
	return TIME_INFINITE;
}

#endif /* GFX_USE_GDISP && GDISP_NEED_IMAGE && GDISP_NEED_IMAGE_NATIVE */
