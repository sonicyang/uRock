UGFXOUTDIR = UGFXLib

UGFXSRCDIR = Middlewares/ugfx \
			 Middlewares/ugfx/src \
			 Middlewares/ugfx/src/gdisp \
			 Middlewares/ugfx/src/gdisp/mcufont \
			 Middlewares/ugfx/src/ginput \
			 Middlewares/ugfx/src/gevent \
			 Middlewares/ugfx/src/gtimer \
			 Middlewares/ugfx/src/gwin \
			 Middlewares/ugfx/src/gdriver \
			 Middlewares/ugfx/src/gqueue \
			 Middlewares/ugfx/src/gox  \
			 Middlewares/ugfx/src/gfile \
			 Middlewares/ugfx/drivers/gdisp/STM32F429iDiscovery \
			 Middlewares/ugfx/drivers/ginput/touch/STMPE811

UGFXINCDIR = Middlewares/ugfx \
			 Middlewares/ugfx/src/gdisp \
			 Middlewares/ugfx/src/gdisp/mcufont \
			 Middlewares/ugfx/src/ginput \
			 Middlewares/ugfx/src/gevent \
			 Middlewares/ugfx/src/gtimer \
			 Middlewares/ugfx/src/gdriver \
			 Middlewares/ugfx/src/gwin \
			 Middlewares/ugfx/src/gos \
			 Middlewares/ugfx/src/gfile \
			 Middlewares/ugfx/drivers/gdisp/STM32F429iDiscovery \
			 Middlewares/ugfx/drivers/ginput/touch/STMPE811

UGFXSRC += $(wildcard $(addsuffix /*.c,$(UGFXSRCDIR))) \
		  $(wildcard $(addsuffix /*.s,$(UGFXSRCDIR)))

UGFXSRC += Middlewares/ugfx/src/gos/gos_freertos.c

INCLUDES += $(addprefix -I,$(UGFXINCDIR))

UGFXOBJS += $(addprefix $(OUTDIR)/$(UGFXOUTDIR)/,$(patsubst %.s,%.o,$(UGFXSRC:.c=.o)))

$(OUTDIR)/$(UGFXOUTDIR)/%.o: %.c
	@echo "   UGFX  |   CC    "$@	
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $(patsubst %.o,%.d,$@) -c $(INCLUDES) $< -o $@

.PHONY: clean-ugfx
clean-ugfx:
	@rm -rf $(OUTDIR)/$(UGFXOUTDIR)
	@echo "Removing UGFX Library Object Files"
