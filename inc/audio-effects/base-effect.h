#ifndef __AUDIO_EFFECT_H__
#define __AUDIO_EFFECT_H__

#include <stdint.h>

#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery_sdram.h"

#include "math.h"

#include "arm_math.h"
#include "core_cmInstr.h"

#include "setting.h"

uint32_t allocateDelayLine();
void releaseDelayLine(uint32_t);

void NormalizeData(volatile uint16_t * pData, q31_t* tData);

void DenormalizeData(q31_t* tData, volatile uint16_t * pData);

struct parameter_t{
    char name[16];
    float value;
    float upperBound;
    float lowerBound;
};

typedef void(*EffectFunc)(q31_t*, void*);
typedef void(*DeleteFunc)(void*);
typedef void(*adjustFunc)(void*, uint8_t*);
typedef void(*getParamFunc)(void*,struct parameter_t*, uint8_t*);

struct Effect_t{
    char name[16];
    EffectFunc func;
    DeleteFunc del;
    adjustFunc adj;
    getParamFunc getParam;
};

#endif //__AUDIO_EFFECT_H__
