#ifndef __EQUALIZER_H__
#define __EQUALIZER_H__

#include "base-effect.h"
#include "lowFreqOsc.h"

struct Equalizer_t{
    struct Effect_t parent;
    struct parameter_t low;
    struct parameter_t mid;
    struct parameter_t high;
    q31_t cache[3]; 

    arm_biquad_casd_df1_inst_q31 S[3];
    q31_t coeffTable[3][20];
    q31_t biquadState[3][16];
};

struct Effect_t* new_Equalizer(struct Equalizer_t* opaque);

#endif //__EQUALIZER_H__
