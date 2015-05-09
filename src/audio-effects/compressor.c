#include "compressor.h"
#include "helper.h"
#include "FreeRTOS.h"

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
    att = (att == 0.0f) ? (0.0f) : expf(-1.0f / (SAMPLING_RATE * 1000 * att));
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
    struct Compressor_t *tmp = (struct Compressor_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_Compressor(void *opaque, uint8_t* values){
    struct Compressor_t *tmp = (struct Compressor_t*)opaque;
    
    LinkPot(&(tmp->threshold), values[0]);  
    LinkPot(&(tmp->attack), values[1]);  
    LinkPot(&(tmp->ratio), values[2]);  
    
    return;
}

void getParam_Compressor(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    struct Compressor_t *tmp = (struct Compressor_t*)opaque;
    *paramNum = 3;
    param[0] = &tmp->threshold;
    param[1] = &tmp->attack;
    param[2] = &tmp->ratio;
    return;
}

struct Effect_t* new_Compressor(){
    struct Compressor_t* tmp = pvPortMalloc(sizeof(struct Compressor_t));
    strcpy(tmp->parent.name, "Compressor");
    tmp->parent.func = Compressor;
    tmp->parent.del = delete_Compressor;
    tmp->parent.adj = adjust_Compressor;
    tmp->parent.getParam = getParam_Compressor;

    tmp->threshold.name = "Threshold";
    tmp->threshold.upperBound = 500.0f;
    tmp->threshold.lowerBound = 100.0f;
    tmp->threshold.value = 100.0f;

    tmp->attack.name = "Attack";
    tmp->attack.upperBound = 0.2f;
    tmp->attack.lowerBound = 0.05f;
    tmp->attack.value = 0.05f;
    
    tmp->ratio.name = "Ratio";
    tmp->ratio.upperBound = 2.0f;
    tmp->ratio.lowerBound = 1.0f;
    tmp->ratio.value = 1.0f;
    
    tmp->env = 0.0f;

    return (struct Effect_t*)tmp;
}
