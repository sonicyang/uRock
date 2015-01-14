#include "volume.h"
#include "helper.h"
#include "math.h"
#include "FreeRTOS.h"

#include "arm_math.h"

void Volume(q31_t* pData, void *opaque){
    struct Volume_t *tmp = (struct Volume_t*)opaque;

    arm_scale_q31(pData, tmp->cache, Q_MULT_SHIFT, pData, SAMPLE_NUM);

    return;
}

void delete_Volume(void *opaque){
    struct Volume_t *tmp = (struct Volume_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_Volume(void *opaque, uint8_t* values){
    struct Volume_t *tmp = (struct Volume_t*)opaque;

    LinkPot(&(tmp->gain), values[0]); 
    tmp->cache = (q31_t)(powf(10, (tmp->gain.value * 0.1f)) * Q_1);

    return;
}

void getParam_Volume(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    struct Volume_t *tmp = (struct Volume_t*)opaque;
    *paramNum = 1;
    param[0] = &tmp->gain;
    return;
}

struct Effect_t* new_Volume(){
    struct Volume_t* tmp = pvPortMalloc(sizeof(struct Volume_t));
    strcpy(tmp->parent.name, "Volume");
    tmp->parent.func = Volume;
    tmp->parent.del = delete_Volume;
    tmp->parent.adj = adjust_Volume;
    tmp->parent.getParam = getParam_Volume;

    strcpy(tmp->gain.name, "Volume");
    tmp->gain.upperBound = 0.0f;
    tmp->gain.lowerBound = -30.0f;
    tmp->gain.value = 0.0f;

    return (struct Effect_t*)tmp;
}

