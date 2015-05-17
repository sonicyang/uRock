#include "lowFreqOsc.h"

float lfoNext(struct LFO_t *opaque){
    float ret;
    
    ret = arm_sin_f32(opaque->cache0 * opaque->ptr);

    opaque->ptr += SAMPLE_NUM;
    if(opaque->ptr > opaque->cache1)
        opaque->ptr -= opaque->cache1;

    return map(ret, 1, -1, opaque->upperBound, opaque->lowerBound);
}

void new_LFO(struct LFO_t *opaque, float u, float l, uint32_t f){
    opaque->ptr = 0;
    opaque->next = lfoNext;
    opaque->feq = f;
    opaque->upperBound = u;
    opaque->lowerBound = l;
    opaque->cache0 = (2 * PI * (float)opaque->feq) / (SAMPLING_RATE * 1000);
    opaque->cache1 = 1000 / (float)opaque->feq * SAMPLING_RATE;
    return;
}

void adjust_LFO_speed(struct LFO_t *opaque, uint32_t f){
    opaque->ptr = ((float)opaque->ptr * opaque->feq) / f;
    opaque->feq = f;
    return;
}
