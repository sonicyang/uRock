#include "audio-effects.h"

int __errno; //For the sake of math.h

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

        *pData = *tData + 128.5f;
    }

    return;
}

void Gain(volatile float* pData, float g){
    register uint32_t i;
    register float gg = powf(10, g);

    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] = pData[i] * gg;
    }

    return;
}

void HardClipping(volatile float* pData, float clip){
    register uint32_t i;
    for(i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] > clip){
            pData[i] = clip;
        }
        else if (pData[i] < -clip){
            pData[i] = -clip;
        }
    }
    return;
}
