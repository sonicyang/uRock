GFXINC +=   $(GFXLIB)
GFXSRC +=	$(GFXLIB)/src/gfx.c

include $(GFXLIB)/src/gos/sys_make.mk
include $(GFXLIB)/src/gqueue/sys_make.mk
include $(GFXLIB)/src/gdisp/sys_make.mk
include $(GFXLIB)/src/gevent/sys_make.mk
include $(GFXLIB)/src/gtimer/sys_make.mk
include $(GFXLIB)/src/gwin/sys_make.mk
include $(GFXLIB)/src/ginput/sys_make.mk
include $(GFXLIB)/src/gadc/sys_make.mk
include $(GFXLIB)/src/gaudio/sys_make.mk
include $(GFXLIB)/src/gmisc/sys_make.mk
include $(GFXLIB)/src/gfile/sys_make.mk
