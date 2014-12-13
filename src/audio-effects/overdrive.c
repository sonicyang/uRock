#include "overdrive.h"
#include "helper.h"

void Overdrive(volatile float* pData, void *opaque){
    struct Overdrive_t *tmp = (struct Overdrive_t*)opaque;
    Gain(pData, tmp->gain.value);
    SoftClipping(pData, 0, tmp->ratio.value);
    Gain(pData, tmp->volume.value);
    return;
}

void delete_Overdrive(void *opaque){
    return;
}

void adjust_Overdrive(void *opaque, uint8_t* values){
    struct Overdrive_t *tmp = (struct Overdrive_t*)opaque;
    
    LinkPot(&(tmp->gain), values[0]);
    LinkPot(&(tmp->volume), values[1]);
    LinkPot(&(tmp->ratio), values[2]);

    return;
}

struct Effect_t* new_Overdrive(struct Overdrive_t* opaque){
    strcpy(opaque->parent.name, "Overdrive");
    opaque->parent.func = Overdrive;
    opaque->parent.del = delete_Overdrive;
    opaque->parent.adj = adjust_Overdrive;

    opaque->gain.upperBound = 15.0f;
    opaque->gain.lowerBound = 0.0f;
    opaque->gain.value = 0.0f;

    opaque->volume.upperBound = 0.0f;
    opaque->volume.lowerBound = -30.0f;
    opaque->volume.value = 0.0f;

    opaque->ratio.upperBound = 1.0f;
    opaque->ratio.lowerBound = 0.5f;
    opaque->ratio.value = 1.0f;

    return (struct Effect_t*)opaque;
}

