This port of DOOM is based on the original license - the free for educational use only version.

See DOOMLIC.TXT for the real license.

This port display a fix sized window of 320 x 200 on your display (it tries to put the display in landscape mode first).

There is no sound currently until the GAUDOUT module is written.
There is no network support currently.

User input is not the best for actually playing yet. Requires Touch or Mouse.

The supplied DOOM1.WAD is the shareware version.
Currently this has been converted into doom1_wad.h for inclusion into the source code (rather than using file IO).
Note that this file is 4M in size so it will take 4Mbytes of Flash to store just the WAD file.

THe RAM requirements are also fairly high with 313K just for the screen image buffers.

This port uses the uGFX streaming interface for high speed access to the video driver.

There are probably a number of C runtime dependancies that shouldn't be there that will get removed
as the port is improved.

Currently tested running under the Win32 uGFX port.
