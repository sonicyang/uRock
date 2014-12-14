#include "flanger.h"
#include "helper.h"

void Flanger(volatile float* pData, void *opaque){
    struct Flanger_t *tmp = (struct Flanger_t*)opaque;
    volatile float bData[256];
    volatile float fData[256];

    int32_t relativeBlock = (tmp->blockPtr - (uint32_t)(tmp->lfo.next(&(tmp->lfo)) / (BLOCK_PERIOD)));
    if(relativeBlock < 0)
        relativeBlock += 400;

    BSP_SDRAM_ReadData(tmp->baseAddress + relativeBlock * SAMPLE_NUM * 4, (uint32_t*)bData, SAMPLE_NUM);

    Copy(fData, pData);
    Combine(pData, bData);

    Gain(bData, tmp->attenuation.value);
    Combine(fData, bData);

    BSP_SDRAM_WriteData(tmp->baseAddress + tmp->blockPtr * SAMPLE_NUM * 4, (uint32_t*)fData, SAMPLE_NUM);

    tmp->blockPtr++;
    if(tmp->blockPtr >= 400)
        tmp->blockPtr = 0;

    return;
}

void delete_Flanger(void *opaque){
    struct Flanger_t *tmp = (struct Flanger_t*)opaque;
    releaseDelayLine(tmp->baseAddress);
    return;
}

void adjust_Flanger(void *opaque, uint8_t* values){
    struct Flanger_t *tmp = (struct Flanger_t*)opaque;
    
    LinkPot(&(tmp->speed), values[0]);  
    LinkPot(&(tmp->attenuation), values[1]);  
    LinkPot(&(tmp->depth), values[2]);  

    adjust_LFO_speed(&(tmp->lfo), tmp->speed.value);
    tmp->lfo.upperBound = tmp->depth.value;

    return;
}


struct Effect_t* new_Flanger(struct Flanger_t* opaque){
    strcpy(opaque->parent.name, "Flanger");
    opaque->parent.func = Flanger;
    opaque->parent.del = delete_Flanger;
    opaque->parent.adj = adjust_Flanger;

    opaque->attenuation.upperBound = -4.0f;
    opaque->attenuation.lowerBound = -30.0f;
    opaque->attenuation.value = -10.0f;

    opaque->speed.upperBound = 500.0f;
    opaque->speed.lowerBound = 5000.0;
    opaque->speed.value = 2000.0f;

    opaque->depth.upperBound = 200.0f;
    opaque->depth.lowerBound = 20.0;
    opaque->depth.value = 25.0f;

    new_LFO(&(opaque->lfo), 25, 5, 1000);

    opaque->blockPtr = 0;
    opaque->baseAddress = allocateDelayLine();

    if(opaque->baseAddress < 0)
        return NULL;

    return (struct Effect_t*)opaque;
}


