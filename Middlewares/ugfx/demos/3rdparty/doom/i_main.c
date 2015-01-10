#include "gfx.h"

#include "doomdef.h"

#include "m_argv.h"
#include "d_main.h"

// Emulate a command line
static const char const *argv[] = { "doom" };

int main(void) {
	gfxInit();

    myargc = sizeof(argv)/sizeof(argv[0]);
    myargv = argv; 
    D_DoomMain();
} 
