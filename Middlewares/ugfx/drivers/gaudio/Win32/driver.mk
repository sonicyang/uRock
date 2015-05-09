# List the required driver.
GFXSRC += $(GFXLIB)/drivers/gaudio/Win32/gaudio_record_lld.c	\
			$(GFXLIB)/drivers/gaudio/Win32/gaudio_play_lld.c

GFXINC += $(GFXLIB)/drivers/gaudio/Win32
GFXLIBS += winmm
