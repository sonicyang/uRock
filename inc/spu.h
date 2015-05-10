#ifndef __SPU_H__
#define __SPU_H__

#include "base-effect.h"

typedef enum {
    NONE,
    VOLUME,
    COMPRESSOR,
    DISTORTION,
    OVERDRIVE,
    DELAY,
    REVERB,
    FLANGER,
    EQULIZER,
    EFFECT_TYPE_NUM
} EffectType_t;

void SignalProcessingUnit(void const * argument);

void attachEffect(uint32_t stage, EffectType_t effectType);
const char *cvtToEffectName(EffectType_t ee);
const struct Effect_t* const retriveStagedEffect(uint32_t stage);

#endif //__SPU_H__
