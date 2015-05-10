#ifndef __SPU_H__
#define __SPU_H__

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
    BACK,
    EFFECT_TYPE_NUM
} EffectType_t;

void SignalProcessingUnit(void const * argument);

void attachNewEffect(uint32_t stage, EffectType_t effectType);
void demolishEffect(uint32_t stage);

#endif //__SPU_H__
