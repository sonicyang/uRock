#include "delay.h"
#include "helper.h"

void Delay(q31_t* pData, void *opaque){
    struct Delay_t *tmp = (struct Delay_t*)opaque;
    q31_t bData[256];

    int32_t relativeBlock = (tmp->blockPtr - (uint32_t)(tmp->delayTime.value / (BLOCK_PERIOD)));
    if(relativeBlock < 0)
        relativeBlock += 400;

    BSP_SDRAM_ReadData(tmp->baseAddress + relativeBlock * SAMPLE_NUM * 4, (uint32_t*)bData, SAMPLE_NUM);

    arm_mult_q31(bData, tmp->cache, bData, SAMPLE_NUM);
    arm_add_q31(pData, bData, pData, SAMPLE_NUM);

    BSP_SDRAM_WriteData(tmp->baseAddress + tmp->blockPtr * SAMPLE_NUM * 4, (uint32_t*)pData, SAMPLE_NUM);

    tmp->blockPtr++;
    if(tmp->blockPtr >= 400)
        tmp->blockPtr = 0;

    return;
}

void delete_Delay(void *opaque){
    struct Delay_t *tmp = (struct Delay_t*)opaque;
    releaseDelayLine(tmp->baseAddress);
    return;
}

void adjust_Delay(void *opaque, uint8_t* values){
    struct Delay_t *tmp = (struct Delay_t*)opaque;
    
    LinkPot(&(tmp->delayTime), values[0]);  
    LinkPot(&(tmp->attenuation), values[1]);  
    arm_fill_q31((q31_t)(powf(10, (tmp->attenuation.value * 0.1f)) * 2147483648), tmp->cache, SAMPLE_NUM);

    return;
}

struct Effect_t* new_Delay(struct Delay_t* opaque){
    strcpy(opaque->parent.name, "Delay");
    opaque->parent.func = Delay;
    opaque->parent.del = delete_Delay;
    opaque->parent.adj = adjust_Delay;

    opaque->attenuation.upperBound = -1.0f;
    opaque->attenuation.lowerBound = -30.0f;
    opaque->attenuation.value = -10.0f;

    opaque->delayTime.upperBound = 500.0f;
    opaque->delayTime.lowerBound = 50.0f;
    opaque->delayTime.value = 50.0f;

    opaque->blockPtr = 0;
    opaque->baseAddress = allocateDelayLine();

    if(opaque->baseAddress < 0)
        return NULL;

    return (struct Effect_t*)opaque;
}


