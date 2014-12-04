#include "audio-effects.h"

void NormalizeData(volatile int8_t * pData){
    uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++){
        *pData = *((uint8_t*)pData) - 128;
    }

    return;
}

void DenormalizeData(volatile int8_t * pData){
    uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++){
        *((uint8_t*)pData) = *pData + 128;
    }

    return;
}

void Gain(volatile int8_t* pData, float g){
    uint32_t i = 0;

    for(; i < SAMPLE_NUM; i++){
        pData[i] *= g;
    }

    return;
}
