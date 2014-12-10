#include "audio-effects.h"

/*
 * The file contains All audio Effects, Parameter are stats for each effect
 * All time base are ms
 * All Volume related base are dB, either absolute or referenced
 *
 * For Absoluted, SAMPLE_MAX is 0dB
 */

int __errno; //For the sake of math.h

inline static uint32_t allocateDelayLine(){
    static uint32_t addr = 0x00100000;
    addr += 0x00100000;
    return addr; 
}

inline static void Combine(volatile float* pData, volatile float* sData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] += sData[i];
    }

    return;
}

inline static void Gain(volatile float* pData, float gain_dB){
    register float multipier = powf(10, (gain_dB / 10.0f));
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] = pData[i] * multipier;
    }

    return;
}

inline static void HardClipping(volatile float* pData, float threshold){
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

inline static void SoftClipping(volatile float* pData, float threshold){
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

inline static uint32_t SDRAM_Delay(volatile float* pData, uint32_t bankptr, volatile float* bData, uint32_t delayblock, uint32_t BaseAddr){
    
    BSP_SDRAM_WriteData(BaseAddr + bankptr * SAMPLE_NUM * 4, (uint32_t*)pData, SAMPLE_NUM);

    delayblock = (bankptr - delayblock) % 400;
    BSP_SDRAM_ReadData(BaseAddr + delayblock * SAMPLE_NUM * 4, (uint32_t*)bData, SAMPLE_NUM);

    bankptr++;

    if(bankptr >= 400)
        bankptr = 0;

    return bankptr;
}

void NormalizeData(volatile uint8_t * pData, volatile float* tData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++, tData++){
        *tData = *pData - SAMPLE_MAX;
    }

    return;
}

void DenormalizeData(volatile float* tData, volatile uint8_t * pData){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++, pData++, tData++){
        if(*tData > SAMPLE_MAX)
            *tData = SAMPLE_MAX;
        else if(*tData < -SAMPLE_MAX)
            *tData = -SAMPLE_MAX;

        *pData = *tData + SAMPLE_MAX + 0.5f;
    }

    return;
}

/*
 * P1 : Atuneation
 */
void Volume(volatile float* pData, struct parameter_t *p){
    Gain(pData, p[0].value);
    return;
}

/*
 * P1 : Atuneation
 */
void Delay(volatile float* pData, struct parameter_t *p){
    static uint32_t baseAddr = DELAY_BANK_0; 
    static uint32_t ptr = 0;
    static volatile float bData[256];

    ptr = SDRAM_Delay(pData, ptr, bData, (uint32_t)(p[0].value / (BLOCK_PREIOD)), baseAddr);
    Gain(bData, p[0].value);
    Combine(pData, bData);

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
