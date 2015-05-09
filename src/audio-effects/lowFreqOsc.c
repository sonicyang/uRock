#include "lowFreqOsc.h"

float lfoNext(struct LFO_t *opaque){
    float ret;
    ret = arm_sin_f32((2 * PI / (float)opaque->period) * opaque->ptr);

    opaque->ptr++;
    if(opaque->ptr > opaque->period)
        opaque->ptr = 0;

    return map(ret, 1, -1, opaque->upperBound, opaque->lowerBound);
}

void new_LFO(struct LFO_t *opaque, float u, float l, uint32_t p){
    opaque->ptr = 0;
    opaque->next = lfoNext;
    opaque->period = p;
    opaque->upperBound = u;
    opaque->lowerBound = l;
    return;
}

void adjust_LFO_speed(struct LFO_t *opaque, uint32_t p){
    opaque->ptr = ((float)opaque->ptr / opaque->period) * p;
    opaque->period = p;
    return;
}
