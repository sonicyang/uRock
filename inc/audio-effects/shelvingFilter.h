#ifndef __SHELVINGFILTER_H__
#define __SHELVINGFILTER_H__

#include "base-effect.h"

struct ShelvingFilter_t{
    struct Effect_t parent;
    struct parameter_t gain;
    q31_t cache[3];
    q31_t xh;
    q31_t c;
    q31_t H0;

    arm_biquad_casd_df1_inst_q31 S[3];
    q31_t biquadState[3][16];
};

struct Effect_t* new_ShelvingFilter();

static struct EffectType_t ShelvingFilterId __attribute__((unused)) = {
    .name = "ShelvingFilter",
    .Init = new_ShelvingFilter
};


#endif //__SHELVINGFILTER_H__
