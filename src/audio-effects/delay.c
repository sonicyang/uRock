#include "delay.h"

void Delay(volatile float* pData, void *opaque){
    struct Delay_t *tmp = (struct Delay_t*)opaque;
    volatile float bData[256];

    tmp->blockPtr = SDRAM_Delay(pData, tmp->blockPtr, bData, (uint32_t)(tmp->delayTime.value / (BLOCK_PREIOD)), tmp->baseAddress);
    Gain(bData, tmp->attenuation.value);
    Combine(pData, bData);

    return;
}
struct Effect_t* new_Delay(struct Delay_t* opaque){
    opaque->attenuation.upperBound = 0.0f;
    opaque->attenuation.lowerBound = -30.0f;
    opaque->attenuation.value = 0.0f;

    opaque->delayTime.upperBound = 500.0f;
    opaque->delayTime.lowerBound = 50.0f;
    opaque->delayTime.value = 50.0f;

    opaque->blockPtr = 0;
    opaque->baseAddress = allocateDelayLine();

    if(opaque->baseAddress < 0)
        return NULL;

    opaque->parent.func = Delay;

    return (struct Effect_t*)opaque;
}

