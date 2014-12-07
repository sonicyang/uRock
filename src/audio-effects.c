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

void Gain(volatile float* pData, struct parameter_t *p){
    register float gg = powf(10, log2f(p[0].value));

    LinearGain(pData, gg);

    return;
}

void Delay(volatile float* pData, struct parameter_t *p){
    static uint32_t baseAddr = DELAY_BANK_0; 
    static uint32_t ptr = 0;
    static volatile float bData[256];

    ptr = SDRAM_Delay(pData, ptr, bData, (uint32_t)(p[0].value / 1.280f), baseAddr);
    Gain(bData, p + 1);
    Combine(pData, bData);

    return;
}


void HardClipping(volatile float* pData, struct parameter_t* p){
    register uint32_t i;
    for(i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] > p[0].value){
            pData[i] = p[0].value;
        }
        else if (pData[i] < -p[0].value){
            pData[i] = -p[0].value;
        }
    }
    return;
}
