USBOUTDIR = USBLib

USBSRCDIR = Middlewares/ST/STM32_USB_Device_Library/Core/Src \
			Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src

USBINCDIR = Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
			Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc

USBSRC += $(wildcard $(addsuffix /*.c,$(USBSRCDIR))) \
		  $(wildcard $(addsuffix /*.s,$(USBSRCDIR)))

INCLUDES += $(addprefix -I,$(USBINCDIR))

USBOBJS += $(addprefix $(OUTDIR)/$(USBOUTDIR)/,$(patsubst %.s,%.o,$(USBSRC:.c=.o)))

$(OUTDIR)/$(USBOUTDIR)/%.o: %.c
	@echo "   USB   |   CC    "$@	
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $@.d -c $(INCLUDES) $< -o $@

.PHONY: clean-usb
clean-usb:
	@rm -rf $(OUTDIR)/$(USBOUTDIR)
	@echo "Removing USB Library Object Files"
