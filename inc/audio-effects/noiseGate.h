#ifndef __NOISEGATE_H__
#define __NOISEGATE_H__

#include "base-effect.h"

struct NoiseGate_t{
    struct Effect_t parent;
    uint32_t counter;
};

struct Effect_t* new_NoiseGate();

static struct EffectType_t NoiseGateId __attribute__((unused)) = {
    .name = "NoiseGate",
    .Init = new_NoiseGate
};


#endif //__NOISEGATE_H__
