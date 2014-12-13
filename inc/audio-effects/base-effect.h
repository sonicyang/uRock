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

void NormalizeData(volatile uint16_t * pData, volatile float* tData);

void DenormalizeData(volatile float* tData, volatile uint16_t * pData);

struct parameter_t{
    char name[16];
    float value;
    float upperBound;
    float lowerBound;
};

typedef void(*EffectFunc)(volatile float*, void*);
typedef void(*DeleteFunc)(void*);
typedef void(*adjustFunc)(void*, uint8_t*);

struct Effect_t{
    char name[16];
    EffectFunc func;
    DeleteFunc del;
    adjustFunc adj;
};

void Combine(volatile float* pData, volatile float* sData);
void Gain(volatile float* pData, float gain_dB);
void HardClipping(volatile float* pData, float threshold);
void SoftClipping(volatile float* pData, float threshold, float ratio);


#endif //__AUDIO_EFFECT_H__
