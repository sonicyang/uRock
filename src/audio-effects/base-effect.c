#include "base-effect.h"

/*
 * The file contains All audio Effects, Parameter are stats for each effect
 * All time base are ms
 * All Volume related base are dB, either absolute or referenced
 *
 * For Absoluted, SAMPLE_MAX is 0dB
 */

int __errno; //For the sake of math.h

uint32_t delayLineStat[7] = {0, 0, 0, 0, 0, 0, 0};

uint32_t allocateDelayLine(){
    register uint32_t i = 0;

    for(; i < 7; i++){
        if(delayLineStat[i] == 0){
            delayLineStat[i] = 1;
            return 0x00100000 * i + 0xD0200000;
        }
    }
    return -1; 
}

void releaseDelayLine(uint32_t address){
    delayLineStat[(address - 0xD0200000) / 0x00100000] = 0;
    return;
}
