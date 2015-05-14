#include "overdrive.h"
#include "helper.h"
#include "FreeRTOS.h"
void Overdrive(q31_t* pData, void *opaque){
    struct Overdrive_t *tmp = (struct Overdrive_t*)opaque;
    register uint32_t i;

    q31_t q31_ratio = tmp->ratio.value * 2147483648; //this is fking Q1.31
    q31_t q31_r_ratio = Q_1 / tmp->ratio.value;

    q31_t q31_gg = SAMPLE_MAX * tmp->gain.value * Q_1;
    q31_t q31_r_gg = 2147483648 / (SAMPLE_MAX * tmp->gain.value); //this is fking Q1.31

    q31_t q31_rg;
    q31_t q31_r_rg; //this is fking Q1.31

    q31_t cacl_tmp;

    arm_mult_q31(&q31_gg, &q31_ratio, &q31_rg, 1);
    arm_scale_q31(&q31_r_gg, q31_r_ratio, Q_MULT_SHIFT, &q31_r_rg, 1);

    for (i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] > q31_rg){
            pData[i] = q31_gg;
        }else if(pData[i] < -q31_rg){
            pData[i] = -q31_gg;
        }else{
            arm_mult_q31(pData + i, &q31_r_rg, &cacl_tmp, 1);
            arm_scale_q31(&cacl_tmp, 0x20000000, Q_MULT_SHIFT, &cacl_tmp, 1);
            cacl_tmp = arm_sin_q31(cacl_tmp);
            arm_mult_q31(&q31_gg, &cacl_tmp, &pData[i], 1);
        }
    }

    arm_scale_q31(pData, tmp->cache, Q_MULT_SHIFT, pData, SAMPLE_NUM);

    return;
}

void delete_Overdrive(void *opaque){
    struct Overdrive_t *tmp = (struct Overdrive_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_Overdrive(void *opaque, uint8_t* values){
    struct Overdrive_t *tmp = (struct Overdrive_t*)opaque;

    LinkPot(&(tmp->gain), values[0]);
    LinkPot(&(tmp->volume), values[1]);
    LinkPot(&(tmp->ratio), values[2]);
    tmp->cache = (q31_t)(powf(10, (tmp->volume.value * 0.1f)) * Q_1);

    return;
}

void getParam_Overdrive(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    struct Overdrive_t *tmp = (struct Overdrive_t*)opaque;
    *paramNum = 3;
    param[0] = &tmp->gain;
    param[1] = &tmp->volume;
    param[2] = &tmp->ratio;
    return;
}

struct Effect_t* new_Overdrive(){
    struct Overdrive_t* tmp = pvPortMalloc(sizeof(struct Overdrive_t));

    strcpy(tmp->parent.name, OverdriveId.name);
    tmp->parent.func = Overdrive;
    tmp->parent.del = delete_Overdrive;
    tmp->parent.adj = adjust_Overdrive;
    tmp->parent.getParam = getParam_Overdrive;

    tmp->gain.name = "Gain";
    tmp->gain.upperBound = 0.35f;
    tmp->gain.lowerBound = 1.0f;
    tmp->gain.value = 1.0f;

    tmp->volume.name = "Volume";
    tmp->volume.upperBound = 0.0f;
    tmp->volume.lowerBound = -30.0f;
    tmp->volume.value = 0.0f;
    tmp->cache = (q31_t)(powf(10, (tmp->volume.value * 0.1f)) * Q_1);

    tmp->ratio.name = "Ratio";
    tmp->ratio.upperBound = 0.85f;
    tmp->ratio.lowerBound = 0.6f;
    tmp->ratio.value = 0.6f;

    return (struct Effect_t*)tmp;
}

