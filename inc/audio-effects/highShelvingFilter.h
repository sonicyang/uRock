#ifndef __HIGHSHELVINGFILTER_H__
#define __HIGHSHELVINGFILTER_H__

#include "base-effect.h"

struct HighShelvingFilter_t{
    struct Effect_t parent;
    struct parameter_t gain;
    q31_t cache[3];
    q31_t xh;
    q31_t c;
    q31_t H0;

    arm_biquad_casd_df1_inst_q31 S[3];
    q31_t biquadState[3][16];
};

struct Effect_t* new_HighShelvingFilter();

static struct EffectType_t HighShelvingFilterId __attribute__((unused)) = {
    .name = "HShelvingFilter",
    .Init = new_HighShelvingFilter
};


#endif //__HIGHSHELVINGFILTER_H__
