#ifndef __EQUALIZER_H__
#define __EQUALIZER_H__

#include "base-effect.h"
#include "eq_img.bmp.h"

struct Equalizer_t{
    struct Effect_t parent;
    struct parameter_t low;
    struct parameter_t mid;
    struct parameter_t high;
    q31_t cache[3]; 

    arm_biquad_casd_df1_inst_q31 S[3];
    q31_t biquadState[3][16];
};

struct Effect_t* new_Equalizer();

static struct EffectType_t EqualizerId __attribute__((unused)) = {
    .name = "Equalizer",
    .image = eq_img,
    .Init = new_Equalizer
};


#endif //__EQUALIZER_H__
