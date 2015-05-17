#ifndef __PEAKFILTER_H__
#define __PEAKFILTER_H__

#include "base-effect.h"

struct PeakFilter_t{
    struct Effect_t parent;
    struct parameter_t gain;
    q31_t cache[3];
    q31_t xh0;
    q31_t xh1;
    q31_t c;
    q31_t d;
    q31_t H0;

    arm_biquad_casd_df1_inst_q31 S[3];
    q31_t biquadState[3][16];
};

struct Effect_t* new_PeakFilter();

static struct EffectType_t PeakFilterId __attribute__((unused)) = {
    .name = "PeakFilter",
    .Init = new_PeakFilter
};


#endif //__PEAKFILTER_H__
