#include "distortion.h"
#include "helper.h"

void Distortion(volatile float* pData, void *opaque){
    struct Distortion_t *tmp = (struct Distortion_t*)opaque;
    Gain(pData, tmp->gain.value);
    HardClipping(pData, 0);
    Gain(pData, tmp->volume.value);
    return;
}

void delete_Distortion(void *opaque){
    return;
}

void adjust_Distortion(void *opaque, uint8_t* values){
    struct Distortion_t *tmp = (struct Distortion_t*)opaque;
    
    LinkPot(&(tmp->gain), values[0]);
    LinkPot(&(tmp->volume), values[1]);

    return;
}

struct Effect_t* new_Distortion(struct Distortion_t* opaque){
    strcpy(opaque->parent.name, "Distortion");
    opaque->parent.func = Distortion;
    opaque->parent.del = delete_Distortion;
    opaque->parent.adj = adjust_Distortion;

    opaque->gain.upperBound = 15.0f;
    opaque->gain.lowerBound = 0.0f;
    opaque->gain.value = 0.0f;

    opaque->volume.upperBound = 0.0f;
    opaque->volume.lowerBound = -30.0f;
    opaque->volume.value = 0.0f;

    return (struct Effect_t*)opaque;
}

