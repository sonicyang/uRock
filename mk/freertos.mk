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

RTOSINCLUDES = $(addprefix -I,$(RTOSINCDIR)) $(addprefix -I,$(HALINCDIR)) -Iinc

RTOSOBJS += $(addprefix $(OUTDIR)/$(RTOSOUTDIR)/,$(patsubst %.s,%.o,$(RTOSSRC:.c=.o)))

INCLUDES += $(addprefix -I,$(RTOSINCDIR))

$(OUTDIR)/$(RTOSOUTDIR)/%.o: %.c
	@echo "   RTOS  |   CC    "$@	
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $@.d -c $(RTOSINCLUDES) $< -o $@

.PHONY: clean-rtos
clean-rtos:
	@rm -rf $(OUTDIR)/$(RTOSOUTDIR)
	@echo "Removing RTOS Object Files"
