#include "audio-effects.h"

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

inline static void LinearGain(volatile float* pData, float g){
    register uint32_t i;

    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] = pData[i] * g;
    }

    return;
}

inline static uint32_t SDRAM_Delay(volatile float* pData, uint32_t bankptr, volatile float* bData, uint32_t delayblock, uint32_t BaseAddr){
    
    delayblock = (bankptr - delayblock) % 400;
    BSP_SDRAM_ReadData(BaseAddr + delayblock * SAMPLE_NUM, (uint32_t*)bData, SAMPLE_NUM / 4);

    BSP_SDRAM_WriteData(BaseAddr + bankptr * SAMPLE_NUM, (uint32_t*)pData, SAMPLE_NUM / 4);

    bankptr++;

    if(bankptr >= 400)
        bankptr = 0;

    return bankptr;
}

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
    register float gg = powf(10, logf(g));

    LinearGain(pData, gg);

    return;
}

void Delay(volatile float* pData, float delay){
    static uint32_t baseAddr = DELAY_BANK_0; 
    static uint32_t ptr = 0;
    static volatile float bData[256];

    ptr = SDRAM_Delay(pData, ptr, bData, (uint32_t)(delay / 1.280f), baseAddr);
    Gain(bData, 0.5);
    Combine(pData, bData);

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

void SoftClipping(volatile float* pData, float clip){
    register uint32_t i;
    for (i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] > clip)
            pData[i] = clip + sqrtf((pData[i] - clip));
        else if (pData[i] < -clip)
            pData[i] = -clip + sqrtf((-pData[i] - clip));
    }
    return;
}


uint8_t pre_status = 0;
uint32_t count = 0;
float pre_r;

void Compressor(volatile float* pData, float threshold, float ratio, float attack){
    float rRatio = 1.0f / ratio;
    float volume = 0.0f;
    float dbvolume;
    float dbthreshold;
    float rate = 1.0f;
    uint8_t status = 0;
    float aData;
    for (int i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] < 0.0f)
            aData = 0.0f - pData[i];
        if (aData > volume){
            volume = aData;
            if (aData > threshold)
                status = 1;
        }
    }
    dbvolume = logf(volume / 127.0f);
    dbthreshold = logf(threshold / 127.0f);
    if (pre_status != status){
        count = 0;
        pre_status = status;
    }
    if (count <= attack){
        rate = count / attack;
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
