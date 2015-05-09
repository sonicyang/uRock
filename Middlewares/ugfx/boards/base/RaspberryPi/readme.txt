This directory contains the interface for the Raspberry Pi framebuffer.
This talks directly to the raspberry pi hardware (not via a linux framebuffer driver).

This graphics interface is software driven - it is not an accelerated interface.

This board definition should work on any operating system that will work on the Raspberry Pi
	eg. Linux, FreeRTOS.

On this board uGFX currently supports:
	- GDISP via the framebuffer driver 

THe following variables may optionally be defined in your gfxconf.h or your makefile...
	- GDISP_LLD_PIXELFORMAT			default = GDISP_PIXELFORMAT_RGB565
	- GDISP_SCREEN_WIDTH			default = 800
	- GDISP_SCREEN_HEIGHT			default = 600

Note that this also provides a Raspberry Pi specific api defined in rpi_mailbox.h to talk
directly to the graphics co-processor from the ARM processor.
