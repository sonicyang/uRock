#ifndef __EQUALIZER_H__
#define __EQUALIZER_H__

#include "base-effect.h"

struct Equalizer_t{
    struct Effect_t parent;
    struct parameter_t high;
    struct parameter_t mid;
    struct parameter_t low;
    q31_t h_xh;
    q31_t h_c;
    q31_t h_H0;

    q31_t m_xh0;
    q31_t m_xh1;
    q31_t m_c;
    q31_t m_d;
    q31_t m_H0;

    q31_t l_xh;
    q31_t l_c;
    q31_t l_H0;

    arm_biquad_casd_df1_inst_q31 S[3];
    q31_t biquadState[3][16];
};

struct Effect_t* new_Equalizer();

static struct EffectType_t EqualizerId __attribute__((unused)) = {
    .name = "HShelvingFilter",
    .Init = new_Equalizer
};


#endif //__EQUALIZER_H__
