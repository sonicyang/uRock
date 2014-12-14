#include "volume.h"
#include "helper.h"

void Volume(volatile float* pData, void *opaque){
    struct Volume_t *tmp = (struct Volume_t*)opaque;
    Gain(pData, tmp->gain.value);
    return;
}

void delete_Volume(void *opaque){
    return;
}

void adjust_Volume(void *opaque, uint8_t* values){
    struct Volume_t *tmp = (struct Volume_t*)opaque;

    LinkPot(&(tmp->gain), values[0]);

    return;
}

struct Effect_t* new_Volume(struct Volume_t* opaque){
    strcpy(opaque->parent.name, "Volume");
    opaque->parent.func = Volume;
    opaque->parent.del = delete_Volume;
    opaque->parent.adj = adjust_Volume;

    opaque->gain.upperBound = 0.0f;
    opaque->gain.lowerBound = -30.0f;
    opaque->gain.value = 0.0f;

    return (struct Effect_t*)opaque;
}

