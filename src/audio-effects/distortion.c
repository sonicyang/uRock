#include "distortion.h"
#include "helper.h"

void Distortion(q31_t* pData, void *opaque){
    struct Distortion_t *tmp = (struct Distortion_t*)opaque;
    uint32_t i;

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

struct Effect_t* new_Distortion(struct Distortion_t* opaque){
    strcpy(opaque->parent.name, "Distortion");
    opaque->parent.func = Distortion;
    opaque->parent.del = delete_Distortion;
    opaque->parent.adj = adjust_Distortion;

    opaque->gain.upperBound = 200.0f;
    opaque->gain.lowerBound = 1.0f;
    opaque->gain.value = 1.0f;

    opaque->volume.upperBound = 0.0f;
    opaque->volume.lowerBound = -30.0f;
    opaque->volume.value = 0.0f;

    return (struct Effect_t*)opaque;
}

