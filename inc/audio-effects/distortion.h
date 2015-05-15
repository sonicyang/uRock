#ifndef __DISTORTION_H__
#define __DISTORTION_H__

#include "base-effect.h"

struct Distortion_t{
    struct Effect_t parent;
    struct parameter_t gain;
    struct parameter_t volume;
    q31_t cache;
};

struct Effect_t* new_Distortion();

static struct EffectType_t DistortionId __attribute__((unused)) = {
    .name = "Distortion",
    .Init = new_Distortion
};


#endif //__DISTORTION_H__
