#include "compressor.h"
#include "helper.h"

void Compressor(q31_t* pData, void *opaque){
    struct Compressor_t *tmp = (struct Compressor_t*)opaque;
    register int i;
    float sum = 0.0f;
    float temp = 0.0f;
    float att, rel;
    float r_q = 1.0f / Q_1;
    float rms = 0, theta;
    float gain = 1.0f;
    att = tmp->attack.value * 1e-3;
    rel = 0.0003;
    att = (att == 0.0f) ? (0.0f) : expf(-1.0f / (SAMPLING_RATE * att));
    rel = expf(-1.0f / (SAMPLING_RATE * rel));

    for (i=0; i<SAMPLE_NUM; i++){
        //arm_scale_q31(pData + i, pData[i], Q_MULT_SHIFT, pData + i, 1);
        temp = pData[i] * r_q;
        temp = temp * temp;
        sum = sum + temp;
    }
    rms = sqrtf(sum / SAMPLE_NUM);
    theta = rms > tmp->env ? att : rel;
    tmp->env = (1.0 - theta) * rms + theta * tmp->env; 
    if (tmp->env > tmp->threshold.value){
        gain = gain - (tmp->env - tmp->threshold.value) * tmp->ratio.value / SAMPLE_MAX;
    }
    
    arm_scale_q31(pData, gain * Q_1, Q_MULT_SHIFT, pData, SAMPLE_NUM);
    return;
}

void delete_Compressor(void *opaque){
    return;
}

void adjust_Compressor(void *opaque, uint8_t* values){
    struct Compressor_t *tmp = (struct Compressor_t*)opaque;
    
    LinkPot(&(tmp->threshold), values[0]);  
    LinkPot(&(tmp->attack), values[1]);  
    LinkPot(&(tmp->ratio), values[2]);  
    
    return;
}

struct Effect_t* new_Compressor(struct Compressor_t* opaque){
    strcpy(opaque->parent.name, "Compressor");
    opaque->parent.func = Compressor;
    opaque->parent.del = delete_Compressor;
    opaque->parent.adj = adjust_Compressor;

    opaque->threshold.upperBound = 500.0f;
    opaque->threshold.lowerBound = 100.0f;
    opaque->threshold.value = 100.0f;

    opaque->attack.upperBound = 0.2f;
    opaque->attack.lowerBound = 0.05f;
    opaque->attack.value = 0.05f;
    
    opaque->ratio.upperBound = 2.0f;
    opaque->ratio.lowerBound = 1.0f;
    opaque->ratio.value = 1.0f;
    
    opaque->env = 0.0f;

    return (struct Effect_t*)opaque;
}
