#include "distortion.h"
#include "helper.h"
#include "FreeRTOS.h"
void Distortion(q31_t* pData, void *opaque){
    struct Distortion_t *tmp = (struct Distortion_t*)opaque;
    arm_scale_q31(pData, tmp->gain.value * Q_1, Q_MULT_SHIFT, pData, SAMPLE_NUM);
    arm_scale_q31(pData, tmp->cache, Q_MULT_SHIFT, pData, SAMPLE_NUM);
    return;
}

void delete_Distortion(void *opaque){
    struct Distortion_t *tmp = (struct Distortion_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_Distortion(void *opaque, uint8_t* values){
    struct Distortion_t *tmp = (struct Distortion_t*)opaque;
    
    LinkPot(&(tmp->gain), values[0]);
    LinkPot(&(tmp->volume), values[1]);
    tmp->cache = (q31_t)(powf(10, (tmp->volume.value * 0.1f)) * Q_1);

    return;
}

void getParam_Distortion(void *opaque, struct parameter_t param[], uint8_t* paramNum){
    struct Distortion_t *tmp = (struct Distortion_t*)opaque;
    *paramNum = 2;
    param[0].value = tmp->gain.value;
    param[1].value = tmp->volume.value;
    return;
}

struct Effect_t* new_Distortion(){
    struct Distortion_t* tmp = pvPortMalloc(sizeof(struct Distortion_t));
    strcpy(tmp->parent.name, "Distortion");
    tmp->parent.func = Distortion;
    tmp->parent.del = delete_Distortion;
    tmp->parent.adj = adjust_Distortion;
    tmp->parent.getParam = getParam_Distortion;

    strcpy(tmp->gain.name, "Gain");
    tmp->gain.upperBound = 200.0f;
    tmp->gain.lowerBound = 10.0f;
    tmp->gain.value = 10.0f;

    strcpy(tmp->volume.name, "Volume");
    tmp->volume.upperBound = 0.0f;
    tmp->volume.lowerBound = -30.0f;
    tmp->volume.value = 0.0f;

    return (struct Effect_t*)tmp;
}

