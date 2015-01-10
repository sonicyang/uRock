GFXINC  += $(GFXLIB)/boards/base/Linux
GFXSRC  +=
GFXDEFS += -DGFX_USE_OS_LINUX=TRUE

include $(GFXLIB)/drivers/multiple/X/gdisp_lld.mk

