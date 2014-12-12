#include "delay.h"
#include "helper.h"

void Delay(volatile float* pData, void *opaque){
    struct Delay_t *tmp = (struct Delay_t*)opaque;
    volatile float bData[256];

    tmp->blockPtr = SDRAM_Delay(pData, tmp->blockPtr, bData, (uint32_t)(tmp->delayTime.value / (BLOCK_PREIOD)), tmp->baseAddress);
    Gain(bData, tmp->attenuation.value);
    Combine(pData, bData);

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


