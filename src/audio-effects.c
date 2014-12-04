#include "audio-effects.h"

void NormalizeData(volatile int8_t * pData, volatile float* tData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++){
        *pData = *((uint8_t*)pData) - 128;
    }

    return;
}

void DenormalizeData(volatile float* tData, volatile int8_t * pData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++){
        *((uint8_t*)pData) = *pData + 128;
    }

    return;
}

void Gain(volatile float* pData, float g){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] = pData[i] * g + 0.5f; //Rounding makes sense
    }

    return;
}
