GFXINC  += $(GFXLIB)/boards/base/Olimex-SAM7EX256-GE12 $(GFXLIB)/boards/base/Olimex-SAM7EX256-GE8
GFXSRC  +=
GFXDEFS += -DGFX_USE_OS_CHIBIOS=TRUE
include $(GFXLIB)/drivers/gdisp/Nokia6610GE12/gdisp_lld.mk
include $(GFXLIB)/drivers/gadc/AT91SAM7/gadc_lld.mk
include $(GFXLIB)/drivers/ginput/dial/GADC/ginput_lld.mk
include $(GFXLIB)/drivers/ginput/toggle/Pal/ginput_lld.mk
include $(GFXLIB)/drivers/gaudio/gadc/driver.mk
