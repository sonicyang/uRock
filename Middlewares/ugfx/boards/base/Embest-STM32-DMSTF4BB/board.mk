GFXINC	+= $(GFXLIB)/boards/base/Embest-STM32-DMSTF4BB
GFXSRC	+=
GFXDEFS += -DGFX_USE_CHIBIOS=TRUE

include $(GFXLIB)/boards/base/Embest-STM32-DMSTF4BB/chibios_board/board.mk
include $(GFXLIB)/drivers/gdisp/SSD2119/gdisp_lld.mk
include $(GFXLIB)/drivers/ginput/touch/STMPE811/ginput_lld.mk

