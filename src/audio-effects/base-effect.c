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
    float ff;

    for (i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] > gg){
            arm_sqrt_f32((pData[i] - gg), &ff);
            pData[i] = gg + ff;
        }else if (pData[i] < -gg){
            arm_sqrt_f32((-pData[i] - gg), &ff);
            pData[i] = -gg + ff;
        }
    }
    return;
}

uint32_t SDRAM_Delay(volatile float* pData, uint32_t bankptr, volatile float* bData, uint32_t delayblock, uint32_t BaseAddr){
    int32_t relativeBlock = (bankptr - delayblock);
    if(relativeBlock < 0)
        relativeBlock += 400;

    BSP_SDRAM_WriteData(BaseAddr + bankptr * SAMPLE_NUM * 4, (uint32_t*)pData, SAMPLE_NUM);

    BSP_SDRAM_ReadData(BaseAddr + relativeBlock * SAMPLE_NUM * 4, (uint32_t*)bData, SAMPLE_NUM);

    bankptr++;

    if(bankptr >= 400)
        bankptr = 0;

    return bankptr;
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

void Compressor(volatile float* pData, struct parameter_t* p){
    static uint8_t pre_status = 0;
    static uint32_t count = 0;
    static float pre_r;
    float rRatio = 1.0f / p[1].value;
    float volume = 0.0f;
    float dbvolume;
    float dbthreshold;
    float rate = 1.0f;
    uint8_t status = 0;
    float aData;
    float attack_block_count = p[1].value / BLOCK_PREIOD;

    for (int i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] < 0.0f)
            aData = 0.0f - pData[i];
        if (aData > volume){
            volume = aData;
            if (aData > p[0].value)
                status = 1;
        }
    }
    dbvolume = logf(volume / 127.0f);
    dbthreshold = logf(p[0].value / 127.0f);
    if (pre_status != status){
        count = 0;
        pre_status = status;
    }
    if (count <= attack_block_count){
        rate = count / attack_block_count;
    }
    float tmp;
    if (status == 1){
        pre_r = dbvolume - dbthreshold;
        tmp = powf(10, -pre_r * rRatio * rate);
    }else{
        tmp = powf(10, -pre_r * rRatio + pre_r * rRatio * rate);
    }
    for (int i = 0; i < SAMPLE_NUM; i++){
        pData[i] = pData[i] * tmp;
    }
    count ++;
}
