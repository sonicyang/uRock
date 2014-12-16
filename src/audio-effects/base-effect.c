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

void Combine(q31_t* pData, q31_t* sData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] += sData[i];
    }

    return;
}

void Copy(q31_t* pData, q31_t* sData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] = sData[i];
    }

    return;
}

void SoftClipping(q31_t* pData, float threshold, float ratio){
    register uint32_t i;
    register float gg = powf(10, (threshold * 0.1f)) * SAMPLE_MAX;
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

void NormalizeData(volatile uint16_t * pData, q31_t* tData){

    arm_offset_q15((q15_t*)pData, (-SAMPLE_MAX), (q15_t*)pData, SAMPLE_NUM);
    arm_q15_to_q31((q15_t*)pData, tData, SAMPLE_NUM);

    return;
}

void DenormalizeData(q31_t* tData, volatile uint16_t * pData){

    arm_q31_to_q15(tData, (q15_t*)pData, SAMPLE_NUM);
    arm_offset_q15((q15_t*)pData, (SAMPLE_MAX), (q15_t*)pData, SAMPLE_NUM);

    return;
}
