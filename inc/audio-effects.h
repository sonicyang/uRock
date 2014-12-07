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

void NormalizeData(volatile uint8_t * pData, volatile float* tData);

void DenormalizeData(volatile float* tData, volatile uint8_t * pData);

struct parameter_t{
    float value;
    float upperBound;
    float lowerBound;
};

typedef void(*EffectFunc)(volatile float*, struct parameter_t*);

struct Effect{
    char name[16];
    EffectFunc func;
    struct parameter_t parameter[4]; //TODO: Use GNU C no length Array
};

void Gain(volatile float* pData, struct parameter_t*);
void HardClipping(volatile float* pData, struct parameter_t*);
void Delay(volatile float* pData, struct parameter_t*);

#endif //__AUDIO_EFFECT_H__
