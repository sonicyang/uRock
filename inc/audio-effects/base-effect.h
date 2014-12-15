#ifndef __AUDIO_EFFECT_H__
#define __AUDIO_EFFECT_H__

#include <stdint.h>

#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery_sdram.h"

#include "math.h"

#define ARM_MATH_CM4
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

struct Effect_t{
    char name[16];
    EffectFunc func;
    DeleteFunc del;
    adjustFunc adj;
};

void Combine(q31_t* pData, q31_t* sData);
void Copy(q31_t* pData, q31_t* sData);
void Gain(q31_t* pData, float gain_dB);
void HardClipping(q31_t* pData, float threshold);
void SoftClipping(q31_t* pData, float threshold, float ratio);


#endif //__AUDIO_EFFECT_H__
