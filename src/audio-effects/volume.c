#include "volume.h"

void Volume(volatile float* pData, void *opaque){
    struct Volume_t *tmp = (struct Volume_t*)opaque;
    Gain(pData, tmp->gain.value);
    return;
}

struct Effect_t* new_Volume(struct Volume_t* opaque){
    opaque->gain.upperBound = 0.0f;
    opaque->gain.lowerBound = -30.0f;
    opaque->gain.value = 0.0f;

    opaque->parent.func = Volume;

    return (struct Effect_t*)opaque;
}

