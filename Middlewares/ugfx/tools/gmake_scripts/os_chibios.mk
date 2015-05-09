#
# This file is subject to the terms of the GFX License. If a copy of
# the license was not distributed with this file, you can obtain one at:
#
#             http://ugfx.org/license.html
#

# See readme.txt for the make API

# Requirements:
#
# CHIBIOS:			The location of the ChibiOS code	eg CHIBIOS=../chibios
# CHIBIOS_PLATFORM	The name of the ChibiOS platform	eg CHIBIOS_PLATFORM=AT91SAM7
# CHIBIOS_PORT		The name of the ChibiOS port		eg CHIBIOS_PORT=GCC/ARM/AT91SAM7
# CHIBIOS_CPUCLASS  The class of the CPU. Only required for ChibiOS v3   eg CHIBIOS_CPUCLASS=ARMCMx
#

# Optional:
#
# CHIBIOS_LDSCRIPT	The name of the loader script		eg CHIBIOS_LDSCRIPT=AT91SAM7X256.ld
# CHIBIOS_BOARD		The name of the ChibiOS board		eg CHIBIOS_BOARD=OLIMEX_SAM7_EX256 - if not specified you must include equivalent code yourself
# CHIBIOS_STM32LIB	Use the STM32 library source for drivers instead of native drivers (yes or no) - default no
# CHIBIOS_VERSION	Which version of ChibiOS is this (2 or 3) - default is 2
# CHIBIOS_PROCESS_STACKSIZE     Size of the ChibiOS process stack. Only useful if the link script supports it - default is 0x400
# CHIBIOS_EXCEPTIONS_STACKSIZE  Size of the ChibiOS exceptopms stack. Only useful if the link script supports it - default is 0x400
#

PATHLIST += CHIBIOS

ifeq ($(CHIBIOS_VERSION),3)
  include $(CHIBIOS)/os/hal/hal.mk
  include $(CHIBIOS)/os/hal/osal/rt/osal.mk
  include $(CHIBIOS)/os/hal/ports/$(CHIBIOS_PLATFORM)/platform.mk
  include $(CHIBIOS)/os/rt/rt.mk
  include $(CHIBIOS)/os/rt/ports/$(CHIBIOS_CPUCLASS)/compilers/GCC/mk/port_$(CHIBIOS_PORT).mk
  ifneq ($(CHIBIOS_BOARD),)
    include $(CHIBIOS)/os/hal/boards/$(CHIBIOS_BOARD)/board.mk
  endif
  LIBPATH += $(CHIBIOS)/os/common/ports/$(CHIBIOS_CPUCLASS)/compilers/GCC
else
  include $(CHIBIOS)/os/hal/hal.mk
  include $(CHIBIOS)/os/hal/platforms/$(CHIBIOS_PLATFORM)/platform.mk
  include $(CHIBIOS)/os/kernel/kernel.mk
  include $(CHIBIOS)/os/ports/$(CHIBIOS_PORT)/port.mk
  ifneq ($(CHIBIOS_BOARD),)
    include $(CHIBIOS)/boards/$(CHIBIOS_BOARD)/board.mk
  endif
endif

ifeq ($(LDSCRIPT),)
  ifneq ($(CHIBIOS_LDSCRIPT),)
    LDSCRIPT= $(PORTLD)/$(CHIBIOS_LDSCRIPT)
  endif
endif

ifneq ($(LDSCRIPT),)
  ifeq ($(CHIBIOS_PROCESS_STACKSIZE),)
    LDFLAGS += -Wl,--defsym=__process_stack_size__=0x400
  else
    LDFLAGS += -Wl,--defsym=__process_stack_size__=$(CHIBIOS_PROCESS_STACKSIZE)
  endif
  ifeq ($(CHIBIOS_EXCEPTIONS_STACKSIZE),)
    LDFLAGS += -Wl,--defsym=__main_stack_size__=0x400
  else
    LDFLAGS += -Wl,--defsym=__main_stack_size__=$(CHIBIOS_EXCEPTIONS_STACKSIZE)
  endif
endif

ifeq ($(CHIBIOS_STM32LIB),yes)
  include $(CHIBIOS)/ext/stm32lib/stm32lib.mk
  SRC     += $(STM32SRC)
  DEFS    += USE_STDPERIPH_DRIVER
  INCPATH += $(STM32INC)
endif

INCPATH += $(PORTINC) $(KERNINC) $(TESTINC) $(OSALINC) \
           $(HALINC) $(PLATFORMINC) $(BOARDINC)
SRC  += $(PORTSRC) \
        $(KERNSRC) \
        $(TESTSRC) \
        $(HALSRC) \
        $(PLATFORMSRC) \
        $(BOARDSRC) \
        $(OSALSRC) \
        $(PORTASM)
