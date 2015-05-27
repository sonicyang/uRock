#ifndef __LOWSHELVINGFILTER_H__
#define __LOWSHELVINGFILTER_H__

#include "base-effect.h"

struct LowShelvingFilter_t{
    struct Effect_t parent;
    struct parameter_t gain;
    q31_t cache[3];
    q31_t xh;
    q31_t c;
    q31_t H0;

    arm_biquad_casd_df1_inst_q31 S[3];
    q31_t biquadState[3][16];
};

struct Effect_t* new_LowShelvingFilter();

static struct EffectType_t LowShelvingFilterId __attribute__((unused)) = {
    .name = "LShelvingFilter",
    .Init = new_LowShelvingFilter
};


#endif //__LOWSHELVINGFILTER_H__
