#ifndef __FLANGER_H__
#define __FLANGER_H__

#include "base-effect.h"
#include "lowFreqOsc.h"

struct Flanger_t{
    struct Effect_t parent;
    struct parameter_t attenuation;
    struct parameter_t speed;
    struct parameter_t depth;
    struct LFO_t lfo;
    uint32_t baseAddress;
    uint32_t blockPtr;
    q31_t cache;
};

struct Effect_t* new_Flanger(struct Flanger_t* opaque);

#endif //__FLANGER_H__
