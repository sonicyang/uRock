#ifndef __AUDIO_EFFECT_H__
#define __AUDIO_EFFECT_H__

#include <stdint.h>

#define SAMPLE_NUM 256

void NormalizeData(volatile int8_t * pData);
void DenormalizeData(volatile int8_t * pData);

typedef void(*effect)(volatile int8_t*, float);

void Gain(volatile int8_t* pData, float g);

#endif //__AUDIO_EFFECT_H__
