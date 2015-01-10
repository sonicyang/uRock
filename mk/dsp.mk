DSPOUTDIR = DSPLib

DSPSRCDIR = Drivers/CMSIS/DSP_Lib/Source/FastMathFunctions \
			Drivers/CMSIS/DSP_Lib/Source/BasicMathFunctions \
			Drivers/CMSIS/DSP_Lib/Source/SupportFunctions \
			Drivers/CMSIS/DSP_Lib/Source/FilteringFunctions

DSPINCDIR = 

DSPSRC += $(wildcard $(addsuffix /*.c,$(DSPSRCDIR))) \
		  $(wildcard $(addsuffix /*.s,$(DSPSRCDIR)))

INCLUDES += $(addprefix -I,$(DSPINCDIR))

DSPOBJS += $(addprefix $(OUTDIR)/$(DSPOUTDIR)/,$(patsubst %.s,%.o,$(DSPSRC:.c=.o)))

$(OUTDIR)/$(DSPOUTDIR)/%.o: %.c
	@echo "   DSP   |   CC    "$@	
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $@.d -c $(INCLUDES) $< -o $@

.PHONY: clean-dsplib
clean-dsplib:
	@rm -rf $(OUTDIR)/$(DSPOUTDIR)
	@echo "Removing DSP Library Object Files"
