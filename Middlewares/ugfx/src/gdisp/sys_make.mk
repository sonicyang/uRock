GFXSRC +=   $(GFXLIB)/src/gdisp/gdisp.c \
			$(GFXLIB)/src/gdisp/fonts.c \
			$(GFXLIB)/src/gdisp/image.c \
			$(GFXLIB)/src/gdisp/image_native.c \
			$(GFXLIB)/src/gdisp/image_gif.c \
			$(GFXLIB)/src/gdisp/image_bmp.c \
			$(GFXLIB)/src/gdisp/image_jpg.c \
			$(GFXLIB)/src/gdisp/image_png.c
			
MFDIR = $(GFXLIB)/src/gdisp/mcufont
include $(GFXLIB)/src/gdisp/mcufont/mcufont.mk
GFXINC += $(MFDIR)
GFXSRC += $(MFSRC)
