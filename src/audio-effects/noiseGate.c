#include "noiseGate.h"
#include "FreeRTOS.h"
#include "helper.h"

void NoiseGate(q31_t* pData, void *opaque){
    struct NoiseGate_t *tmp = (struct NoiseGate_t*)opaque;
    q31_t result;
    q31_t threshold = tmp->threshold.value * Q_1;
    register int i;
    q31_t c = 0;
    // arm_rms_q31(pData, SAMPLE_NUM, &result);
    for(i = 0; i < SAMPLE_NUM; i++){
        if(pData[i] < threshold){
            c++;
        }
    }
    if (c == SAMPLE_NUM){
        tmp->counter += 1;
    }else{
        tmp->counter = 0;
    }
    if (tmp->counter > 256){
        arm_fill_q31(0, pData, SAMPLE_NUM);
    }
    return;
}

void delete_NoiseGate(void *opaque){
    struct NoiseGate_t *tmp = (struct NoiseGate_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_NoiseGate(void *opaque, uint8_t* values){
    struct NoiseGate_t *tmp = (struct NoiseGate_t*)opaque;

    LinkPot(&(tmp->threshold), values[0]);
    return;
}

void getParam_NoiseGate(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    struct NoiseGate_t *tmp = (struct NoiseGate_t*)opaque;
    *paramNum = 1;
    param[0] = &tmp->threshold;
    return;
}

struct Effect_t* new_NoiseGate(){
    struct NoiseGate_t* tmp = pvPortMalloc(sizeof(struct NoiseGate_t));

    strcpy(tmp->parent.name, NoiseGateId.name);
    tmp->parent.func = NoiseGate;
    tmp->parent.del = delete_NoiseGate;
    tmp->parent.adj = adjust_NoiseGate;
    tmp->parent.getParam = getParam_NoiseGate;

    tmp->threshold.name = "Threshold";
    tmp->threshold.upperBound = 8192.0f;
    tmp->threshold.lowerBound = 0.0f;
    tmp->threshold.value = 1.0f;

    tmp->counter = 0;
    return (struct Effect_t*)tmp;
}

