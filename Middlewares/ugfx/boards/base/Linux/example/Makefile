#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!
#
##############################################################################################
#
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#

##############################################################################################
# Start of default section
#

TRGT = 
CC   = $(TRGT)gcc
AS   = $(TRGT)gcc -x assembler-with-cpp

# List all default C defines here, like -D_DEBUG=1
DDEFS = 

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = -lX11 -pthread -lrt

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

# Define project name here
PROJECT = ugfx

# Imported source files and paths for uGFX
GFXLIB = ../ugfx
include ${GFXLIB}/gfx.mk
include ${GFXLIB}/boards/base/Linux/board.mk

# Where is our source code - alter these for your project.
# Either just include the demo makefile or add your own definitions
include $(GFXLIB)/demos/modules/gdisp/basics/demo.mk
#MYFILES = my-project-directory
#MYCSRC  = $(MYFILES)/main.c
#MYDEFS  =

# List all user C define here, like -D_DEBUG=1
UDEFS = $(MYDEFS) $(GFXDEFS)

# Define ASM defines here
UADEFS =

# List C source files here
SRC  = $(GFXSRC) \
       $(MYCSRC)

# List ASM source files here
ASRC =

# List all user directories here
UINCDIR = $(MYFILES) $(GFXINC)

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

# Define optimisation level here
OPT = -ggdb -O0 -fomit-frame-pointer

#
# End of user defines
##############################################################################################

INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))
DEFS    = $(DDEFS) $(UDEFS)
ADEFS   = $(DADEFS) $(UADEFS)
OBJS    = $(ASRC:.s=.o) $(SRC:.c=.o)
LIBS    = $(DLIBS) $(ULIBS)

ASFLAGS = -Wa,-amhls=$(<:.s=.lst) $(ADEFS)
CPFLAGS = $(OPT) -Wall -Wextra -Wstrict-prototypes -fverbose-asm $(DEFS) 

ifeq ($(HOST_OSX),yes)
  ifeq ($(OSX_SDK),)
    OSX_SDK = /Developer/SDKs/MacOSX10.7.sdk
  endif
  ifeq ($(OSX_ARCH),)
    OSX_ARCH = -mmacosx-version-min=10.3 -arch i386
  endif

  CPFLAGS += -isysroot $(OSX_SDK) $(OSX_ARCH)
  LDFLAGS = -Wl -Map=$(PROJECT).map,-syslibroot,$(OSX_SDK),$(LIBDIR)
  LIBS += $(OSX_ARCH)
else
  # Linux, or other
  CPFLAGS += -m32 -Wa,-alms=$(<:.c=.lst)
  LDFLAGS = -m32 -Wl,-Map=$(PROJECT).map,--cref,--no-warn-mismatch $(LIBDIR)
endif

# Generate dependency information
CPFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#

all: $(OBJS) $(PROJECT)

%.o : %.c
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

%.o : %.s
	$(AS) -c $(ASFLAGS) $< -o $@

$(PROJECT): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

gcov:
	-mkdir gcov
	$(COV) -u $(subst /,\,$(SRC))
	-mv *.gcov ./gcov

clean:                                      
	-rm -f $(OBJS)
	-rm -f $(PROJECT)
	-rm -f $(PROJECT).map
	-rm -f $(SRC:.c=.c.bak)
	-rm -f $(SRC:.c=.lst)
	-rm -f $(ASRC:.s=.s.bak)
	-rm -f $(ASRC:.s=.lst)
	-rm -fR .dep

#
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
