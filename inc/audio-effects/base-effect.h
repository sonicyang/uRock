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

int32_t allocateDelayLine();
void releaseDelayLine(uint32_t);

void NormalizeData(volatile uint8_t * pData, volatile float* tData);

void DenormalizeData(volatile float* tData, volatile uint8_t * pData);

struct parameter_t{
    char name[16];
    float value;
    float upperBound;
    float lowerBound;
};

typedef void(*EffectFunc)(volatile float*, void*);
typedef void(*DeleteFunc)(void*);

struct Effect_t{
    char name[16];
    EffectFunc func;
    DeleteFunc del;
};

void Combine(volatile float* pData, volatile float* sData);
void Gain(volatile float* pData, float gain_dB);
void HardClipping(volatile float* pData, float threshold);
void SoftClipping(volatile float* pData, float threshold);
uint32_t SDRAM_Delay(volatile float* pData, uint32_t bankptr, volatile float* bData, uint32_t delayblock, uint32_t BaseAddr);

void Compressor(volatile float* pData, struct parameter_t* p);

#endif //__AUDIO_EFFECT_H__
