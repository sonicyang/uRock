#include "distortion.h"
#include "helper.h"
#include "FreeRTOS.h"

static const char* effectName = "Distortion";
static uint32_t effectID;

__attribute__((constructor))
void init_Distortion(uint32_t givenID, char* name){
    effectID = givenID;
    strcpy(name, effectName);

    return;
}

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

void getParam_Distortion(void *opaque, struct parameter_t *param[], uint8_t* paramNum){
    struct Distortion_t *tmp = (struct Distortion_t*)opaque;
    *paramNum = 2;
    param[0] = &tmp->gain;
    param[1] = &tmp->volume;
    return;
}

struct Effect_t* new_Distortion(){
    struct Distortion_t* tmp = pvPortMalloc(sizeof(struct Distortion_t));

    strcpy(tmp->parent.name, DistortionId.name);
    tmp->parent.func = Distortion;
    tmp->parent.del = delete_Distortion;
    tmp->parent.adj = adjust_Distortion;
    tmp->parent.getParam = getParam_Distortion;

    tmp->gain.name = "Gain";
    tmp->gain.upperBound = 10.0f;
    tmp->gain.lowerBound = 2.0f;
    tmp->gain.value = 2.0f;

    tmp->volume.name = "Volume";
    tmp->volume.upperBound = 0.0f;
    tmp->volume.lowerBound = -30.0f;
    tmp->volume.value = 0.0f;
    tmp->cache = (q31_t)(powf(10, (tmp->volume.value * 0.1f)) * Q_1);

    return (struct Effect_t*)tmp;
}

