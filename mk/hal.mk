HALOUTDIR = HAL

HALSRCDIR = Drivers/STM32F4xx_HAL_Driver/Src

HALINCDIR = Drivers/STM32F4xx_HAL_Driver/Inc \
		    Drivers/CMSIS/Device/ST/STM32F4xx/Include \
			Drivers/CMSIS/Include \

HALSRC += $(wildcard $(addsuffix /*.c,$(HALSRCDIR))) \
		  $(wildcard $(addsuffix /*.s,$(HALSRCDIR)))

INCLUDES += $(addprefix -I,$(HALINCDIR))

HALOBJS += $(addprefix $(OUTDIR)/$(HALOUTDIR)/, $(patsubst %.s,%.o,$(HALSRC:.c=.o)))

$(OUTDIR)/$(HALOUTDIR)/%.o: %.c
	@echo "   HAL   |   CC    "$@	
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $@.d -c $(INCLUDES) $< -o $@

.PHONY: clean-hal
clean-hal:
	@rm -rf $(OUTDIR)/$(HALOUTDIR)
	@echo "Removing HAL Object Files"
