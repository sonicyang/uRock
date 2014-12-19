RTOSOUTDIR = RTOS

RTOSSRCDIR = Middlewares/Third_Party/FreeRTOS/Source \
			 Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
			 Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS

RTOSINCDIR =  Middlewares/Third_Party/FreeRTOS/Source/include \
			  Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
			  Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS

RTOSSRC += $(wildcard $(addsuffix /*.c,$(RTOSSRCDIR))) \
		  $(wildcard $(addsuffix /*.s,$(RTOSSRCDIR)))

RTOSSRC += Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c

INCLUDES += $(addprefix -I,$(RTOSINCDIR))

RTOSOBJS += $(addprefix $(OUTDIR)/$(RTOSOUTDIR)/,$(patsubst %.s,%.o,$(RTOSSRC:.c=.o)))

$(OUTDIR)/$(RTOSOUTDIR)/%.o: %.c
	@echo "   RTOS  |   CC    "$@	
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $@.d -c $(INCLUDES) $< -o $@

.PHONY: clean-rtos
clean-rtos:
	@rm -rf $(OUTDIR)/$(RTOSOUTDIR)
	@echo "Removing RTOS Object Files"
