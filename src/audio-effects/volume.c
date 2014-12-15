#include "volume.h"
#include "helper.h"
#include "math.h"

#define ARM_MATH_CM4
#include "arm_math.h"

void Volume(q31_t* pData, void *opaque){
    struct Volume_t *tmp = (struct Volume_t*)opaque;

    arm_mult_q31(pData, tmp->cache, pData, SAMPLE_NUM);

    return;
}

void delete_Volume(void *opaque){
    return;
}

void adjust_Volume(void *opaque, uint8_t* values){
    struct Volume_t *tmp = (struct Volume_t*)opaque;

    LinkPot(&(tmp->gain), values[0]); 
    arm_fill_q31((q31_t)(powf(10, (tmp->gain.value * 0.1f)) * 2147483648), tmp->cache, SAMPLE_NUM);

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

