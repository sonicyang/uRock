#include "flanger.h"
#include "helper.h"

void Flanger(q31_t* pData, void *opaque){
    struct Flanger_t *tmp = (struct Flanger_t*)opaque;
    q31_t bData;
    q31_t fData[256];
    uint32_t i;
    int32_t relativeBlock;

    arm_copy_q31(pData, fData, SAMPLE_NUM);
    arm_scale_q31(pData, (q31_t)(0.5 * Q_1), Q_MULT_SHIFT, pData, SAMPLE_NUM);

    for (i = 0; i < SAMPLE_NUM; i++) {
        relativeBlock = (tmp->blockPtr * SAMPLE_NUM + i - (uint32_t)(tmp->lfo.next(&(tmp->lfo))));

        if(relativeBlock < 0)
            relativeBlock += 400 * SAMPLE_NUM;

        BSP_SDRAM_ReadData(tmp->baseAddress + relativeBlock * 4, (uint32_t*)&bData, 1);

        arm_scale_q31(&bData, (q31_t)(0.2 * Q_1), Q_MULT_SHIFT, &bData, 1);
        arm_add_q31(pData + i, &bData, pData + i, 1);

        arm_scale_q31(&bData, tmp->cache, Q_MULT_SHIFT, &bData, 1);
        arm_add_q31(fData + i, &bData, fData + i, 1);
    }

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

    tmp->cache = (q31_t)(powf(10, (tmp->attenuation.value * 0.1f)) * 2 * Q_1); //saving memory

    adjust_LFO_speed(&(tmp->lfo), tmp->speed.value / SAMPLE_PERIOD);
    tmp->lfo.upperBound = tmp->depth.value / SAMPLE_PERIOD;

    return;
}

void getParam_Flanger(void *opaque, struct parameter_t param[], uint8_t* paramNum){
    struct Flanger_t *tmp = (struct Flanger_t*)opaque;
    *paramNum = 3;
    param[0].value = tmp->attenuation.value;
    param[1].value = tmp->speed.value;
    param[2].value = tmp->depth.value;
    return;
}


struct Effect_t* new_Flanger(struct Flanger_t* opaque){
    strcpy(opaque->parent.name, "Flanger");
    opaque->parent.func = Flanger;
    opaque->parent.del = delete_Flanger;
    opaque->parent.adj = adjust_Flanger;
    opaque->parent.getParam = getParam_Flanger;

    opaque->attenuation.upperBound = -2.0f;
    opaque->attenuation.lowerBound = -30.0f;
    opaque->attenuation.value = -30.0f;

    opaque->speed.upperBound = 500.0f;
    opaque->speed.lowerBound = 5000.0;
    opaque->speed.value = 2000.0f;

    opaque->depth.upperBound = 200.0f;
    opaque->depth.lowerBound = 10.0;
    opaque->depth.value = 25.0f;

    new_LFO(&(opaque->lfo), 25 / SAMPLE_PERIOD, 10 / SAMPLE_PERIOD, 1000 / SAMPLE_PERIOD);

    opaque->blockPtr = 0;
    opaque->baseAddress = allocateDelayLine();

    if(opaque->baseAddress < 0)
        return NULL;

    return (struct Effect_t*)opaque;
}


