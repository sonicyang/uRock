PROJECT = main

TOOLDIR = tool

OUTDIR = build
EXECUTABLE = $(OUTDIR)/$(PROJECT).elf
BIN_IMAGE = $(OUTDIR)/$(PROJECT).bin
HEX_IMAGE = $(OUTDIR)/$(PROJECT).hex
MAP_FILE = $(OUTDIR)/$(PROJECT).map
LIST_FILE = $(OUTDIR)/$(PROJECT).lst

# Toolchain configurations
CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size

# Cortex-M4 implements the ARMv7E-M architecture
CPU = cortex-m4
CFLAGS = -mcpu=$(CPU) -march=armv7e-m -mtune=cortex-m4
CFLAGS += -mlittle-endian -mthumb
# Need study
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# Basic configurations
CFLAGS += -g3 -std=c99 -Wall  \
		  -DUSER_NAME=\"$(USER)\"

# Optimizations
CFLAGS += -O0 -ffast-math \
		  -ffunction-sections -fdata-sections \
		  -fno-common \
		  --param max-inline-insns-single=1000

# specify STM32F429
CFLAGS += -DSTM32F429xx -DARM_MATH_CM4 -D__FPU_PRESENT

# to run from FLASH
CFLAGS += -DVECT_TAB_FLASH
LDFLAGS += -Wl,--gc-sections -Wl,-Map=$(MAP_FILE) -TSTM32F429I_DISCO/STM32F429ZI_FLASH.ld

#files
SRCDIR = src \
		 src/audio-effects \

INCDIR = inc \
		 inc/audio-effects \

SRC += $(wildcard $(addsuffix /*.c,$(SRCDIR))) \
	  $(wildcard $(addsuffix /*.s,$(SRCDIR)))

OBJS += $(addprefix $(OUTDIR)/,$(patsubst %.s,%.o,$(SRC:.c=.o)))

INCLUDES = $(addprefix -I,$(INCDIR))

DEP = $(OBJ:.o=.o.d)

MAKDIR = mk
MAK = $(wildcard $(MAKDIR)/*.mk)

all: $(BIN_IMAGE)

include $(MAK)

$(BIN_IMAGE): $(EXECUTABLE)
	@$(OBJCOPY) -O binary $^ $@
	@$(OBJCOPY) -O ihex $^ $(HEX_IMAGE)
	@$(OBJDUMP) -h -S -D $^ > $(LIST_FILE)
	@echo "   ALL   |  OBJCOPY   "$@	
	@echo "   ALL   |  OBJCOPY   "$(HEX_IMAGEX)	
	@echo "   ALL   |  OBJDUMP   "$(LIST_FILE)
	@$(SIZE) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJS) $(HALOBJS) $(BSPOBJS) $(RTOSOBJS) $(DSPOBJS) $(FATOBJS) $(USBOBJS) $(UGFXOBJS)
	@echo "   ALL   |   LD    "$@	
	@$(CROSS_COMPILE)gcc $(CFLAGS) $(LDFLAGS) -lc -lgcc -lnosys -lm -o $@ $^

$(OUTDIR)/%.o: %.c
	@echo "   MAIN  |   CC    "$@	
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $@.d -c $(INCLUDES) $< -o $@

$(OUTDIR)/%.o: %.s
	@echo "   MAIN  |   AS    "$@	
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $@.d -c $(INCLUDES) $< -o $@

flash: $(EXECUTABLE)
	st-flash write $(BIN_IMAGE) 0x8000000

openocd_flash: $(EXECUTABLE)
	openocd \
	-f board/stm32f429discovery.cfg \
	-c "init" \
	-c "reset init" \
	-c "flash probe 0" \
	-c "flash info 0" \
	-c "flash write_image erase $(BIN_IMAGE)  0x08000000" \
	-c "reset run" -c shutdown

.PHONY: clean
clean:
	@rm -rf $(BIN_IMAGE)
	@rm -rf $(HEX_IMAGE)
	@rm -rf $(LIST_FILE)
	@rm -rf $(OUTDIR)/src/*
	@echo "Removing Project Object Files"

.PHONY: clean-all
clean-all:
	@rm -rf $(OUTDIR)/*
	@echo "Removing All Object Files"

dbg: $(EXECUTABLE)
	openocd -f board/stm32f429discovery.cfg >/dev/null & \
    echo $$! > $(OUTDIR)/openocd_pid && \
    $(CROSS_COMPILE)gdb -x $(TOOLDIR)/gdbscript && \
    cat $(OUTDIR)/openocd_pid |`xargs kill 2>/dev/null || test true` && \
    rm -f $(OUTDIR)/openocd_pid

-include $(DEP)
