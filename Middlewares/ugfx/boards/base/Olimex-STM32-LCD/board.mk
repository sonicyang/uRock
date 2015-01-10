GFXINC	+= $(GFXLIB)/boards/base/Olimex-STM32-LCD
GFXSRC	+=
GFXDEFS += -DGFX_USE_CHIBIOS=TRUE

include $(GFXLIB)/boards/base/Olimex-STM32-LCD/chibios_board/board.mk
include $(GFXLIB)/drivers/gdisp/ILI9320/gdisp_lld.mk
include $(GFXLIB)/drivers/ginput/touch/MCU/ginput_lld.mk

