#include "noiseGate.h"
#include "FreeRTOS.h"
#include "helper.h"

void NoiseGate(q31_t* pData, void *opaque){
    struct NoiseGate_t *tmp = (struct NoiseGate_t*)opaque;
    q31_t buf[SAMPLE_NUM];
    q31_t threshold = 107 * Q_1;
    register int i;
    q31_t c = 0;
    // arm_rms_q31(pData, SAMPLE_NUM, &result);
    arm_abs_q31(pData, buf, SAMPLE_NUM);
    for(i = 0; i < SAMPLE_NUM; i++){
        if(buf[i] < threshold){
            c++;
        }
    }
    if (c >= SAMPLE_NUM / 2){
        tmp->counter += 1;
    }else{
        tmp->counter = 0;
    }
    if (tmp->counter > 32){
        arm_fill_q31(0, pData, SAMPLE_NUM);
    }
    return;
}

void delete_NoiseGate(void *opaque){
    struct NoiseGate_t *tmp = (struct NoiseGate_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_NoiseGate(void *opaque, uint8_t* values){
    return;
}

void getParam_NoiseGate(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    *paramNum = 0;
    return;
}

struct Effect_t* new_NoiseGate(){
    struct NoiseGate_t* tmp = pvPortMalloc(sizeof(struct NoiseGate_t));

    strcpy(tmp->parent.name, NoiseGateId.name);
    tmp->parent.func = NoiseGate;
    tmp->parent.del = delete_NoiseGate;
    tmp->parent.adj = adjust_NoiseGate;
    tmp->parent.getParam = getParam_NoiseGate;

    tmp->counter = 0;
    return (struct Effect_t*)tmp;
}

