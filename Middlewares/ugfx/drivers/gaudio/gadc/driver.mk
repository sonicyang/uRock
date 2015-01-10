# List the required driver.
GFXSRC += $(GFXLIB)/drivers/gaudio/gadc/gaudio_record_lld.c

# Required include directories
GFXINC += $(GFXLIB)/drivers/gaudio/gadc

# Make sure the GADC sub-system is turned on
GFXDEFS += -DGFX_USE_GADC=GAUDIO_NEED_RECORD

