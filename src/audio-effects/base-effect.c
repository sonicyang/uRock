#include "base-effect.h"

/*
 * The file contains All audio Effects, Parameter are stats for each effect
 * All time base are ms
 * All Volume related base are dB, either absolute or referenced
 *
 * For Absoluted, SAMPLE_MAX is 0dB
 */

int __errno; //For the sake of math.h

uint32_t delayLineStat[7] = {0, 0, 0, 0, 0, 0, 0};

uint32_t allocateDelayLine(){
    register uint32_t i = 0;

    for(; i < 7; i++){
        if(delayLineStat[i] == 0){
            delayLineStat[i] = 1;
            return 0x00100000 * i + 0xD0200000;
        }
    }
    return -1; 
}

void releaseDelayLine(uint32_t address){
    delayLineStat[(address - 0xD0200000) / 0x00100000] = 0;
    return;
}

void Combine(volatile float* pData, volatile float* sData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] += sData[i];
    }

    return;
}

void Gain(volatile float* pData, float gain_dB){
    register float multipier = powf(10, (gain_dB / 10.0f));
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] = pData[i] * multipier;
    }

    return;
}

void HardClipping(volatile float* pData, float threshold){
    register uint32_t i;
    register float gg = powf(10, (threshold / 10.0f)) * SAMPLE_MAX;

    for(i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] > gg){
            pData[i] = gg;
        }else if (pData[i] < -gg){
            pData[i] = -gg;
        }
    }

    return;
}

void SoftClipping(volatile float* pData, float threshold){
    register uint32_t i;
    register float gg = powf(10, (threshold / 10.0f)) * SAMPLE_MAX;
    float ratio = 1;
    float tmp = 1 / (ratio * gg);
    for (i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] > ratio * gg){
            pData[i] = gg;
        }
        else if (pData[i] < -ratio * gg){
            pData[i] = -gg;
        }
        else{
            pData[i] = arm_sin_f32(PI * pData[i] * 0.5 * tmp) * gg;
        }
    }

    return;
}

void NormalizeData(volatile uint16_t * pData, volatile float* tData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++, tData++){
        *tData = *pData - SAMPLE_MAX;
    }

    return;
}

void DenormalizeData(volatile float* tData, volatile uint16_t * pData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++, tData++){
        
        if(*tData > SAMPLE_MAX)
            *tData = SAMPLE_MAX + 0.5f;
        else if(*tData < -SAMPLE_MAX)
            *tData = -SAMPLE_MAX - 0.5f;
       

        *pData = *tData + SAMPLE_MAX;
    }

    return;
}
