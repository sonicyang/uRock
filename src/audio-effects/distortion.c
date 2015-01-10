#include "distortion.h"
#include "helper.h"

void Distortion(q31_t* pData, void *opaque){
    struct Distortion_t *tmp = (struct Distortion_t*)opaque;
    arm_scale_q31(pData, tmp->gain.value * Q_1, Q_MULT_SHIFT, pData, SAMPLE_NUM);
    arm_scale_q31(pData, tmp->cache, Q_MULT_SHIFT, pData, SAMPLE_NUM);
    return;
}

void delete_Distortion(void *opaque){
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

struct Effect_t* new_Distortion(struct Distortion_t* opaque){
    strcpy(opaque->parent.name, "Distortion");
    opaque->parent.func = Distortion;
    opaque->parent.del = delete_Distortion;
    opaque->parent.adj = adjust_Distortion;
    opaque->parent.getParam = getParam_Distortion;

    opaque->gain.upperBound = 200.0f;
    opaque->gain.lowerBound = 10.0f;
    opaque->gain.value = 10.0f;

    opaque->volume.upperBound = 0.0f;
    opaque->volume.lowerBound = -30.0f;
    opaque->volume.value = 0.0f;

    return (struct Effect_t*)opaque;
}

