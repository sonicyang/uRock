#include "audio-effects.h"

void NormalizeData(volatile uint8_t * pData, volatile float* tData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++, tData++){
        *tData = *pData - 128;
    }

    return;
}

void DenormalizeData(volatile float* tData, volatile uint8_t * pData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++, tData++){
        if(*tData > 127)
            *tData = 127;
        else if(*tData < -128)
            *tData = -128;

        *pData = *tData + 128;
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
