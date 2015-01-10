Copy these files into your own project directory and alter them to suite.

Notes:

1/ This makefile uses the MINGW compiler tool chain and was run using the cygwin make.
2/ At the top of the Makefile is the define USE_CHIBIOS. Win32 can build uGFX for either
	native Win32 (the default) or for the ChibiOS simulator.
3/ The files chconf.h and halconf.h are only needed if compiling for the ChibiOS simulator.
4/ Look at the MYFILES definition and the MYCSRC definition.
