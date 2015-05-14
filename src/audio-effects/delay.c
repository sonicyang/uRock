#include "delay.h"
#include "helper.h"
#include "FreeRTOS.h"

void Delay(q31_t* pData, void *opaque){
    struct Delay_t *tmp = (struct Delay_t*)opaque;
    q31_t bData[256];

    int32_t relativeBlock = (tmp->blockPtr - (uint32_t)(tmp->delayTime.value / (BLOCK_PERIOD)));
    if(relativeBlock < 0)
        relativeBlock += 400;

    BSP_SDRAM_ReadData(tmp->baseAddress + relativeBlock * SAMPLE_NUM * 4, (uint32_t*)bData, SAMPLE_NUM);

    arm_scale_q31(bData, tmp->cache, Q_MULT_SHIFT,  bData, SAMPLE_NUM);
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
    vPortFree(tmp); 
    return;
}

void adjust_Delay(void *opaque, uint8_t* values){
    struct Delay_t *tmp = (struct Delay_t*)opaque;
    
    LinkPot(&(tmp->delayTime), values[0]);  
    LinkPot(&(tmp->attenuation), values[1]);  
    tmp->cache = (q31_t)(powf(10, (tmp->attenuation.value * 0.1f)) * Q_1);

    return;
}

void getParam_Delay(void *opaque, struct parameter_t *param[], uint8_t* paramNum){
    struct Delay_t *tmp = (struct Delay_t*)opaque;
    *paramNum = 2;
    param[0] = &tmp->attenuation;
    param[1] = &tmp->delayTime;
    return;
}

struct Effect_t* new_Delay(){
    struct Delay_t* tmp = pvPortMalloc(sizeof(struct Delay_t));
    strcpy(tmp->parent.name, "Delay");
    tmp->parent.func = Delay;
    tmp->parent.del = delete_Delay;
    tmp->parent.adj = adjust_Delay;
    tmp->parent.getParam = getParam_Delay;

    tmp->attenuation.name = "Attenuation";
    tmp->attenuation.upperBound = -5.0f;
    tmp->attenuation.lowerBound = -30.0f;
    tmp->attenuation.value = -10.0f;
    tmp->cache = (q31_t)(powf(10, (tmp->attenuation.value * 0.1f)) * Q_1);

    tmp->delayTime.name = "Delay";
    tmp->delayTime.upperBound = 500.0f;
    tmp->delayTime.lowerBound = 50.0f;
    tmp->delayTime.value = 50.0f;

    tmp->blockPtr = 0;
    tmp->baseAddress = allocateDelayLine();

    if(tmp->baseAddress < 0)
        return NULL;

    return (struct Effect_t*)tmp;
}


