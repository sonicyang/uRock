#include "shelvingFilter.h"
#include "helper.h"
#include "FreeRTOS.h"

/*q31_t arm_tan_q31(q31_t x){
    return ((float)(arm_sin_q31(x) / Q_1)) /
           ((float)(arm_cos_q31(x) / Q_1)) * Q_1;

}*/
void ShelvingFilter(q31_t* pData, void *opaque){
    struct ShelvingFilter_t *tmp = (struct ShelvingFilter_t*)opaque;
    register uint32_t i;
    /*q31_t V0 = opaque->gain.value; // pow10(opaque->G.value * 0.05 * Q_1) ;*/
    /*q31_t H0 = V0 - 1;*/

    q31_t calc_tmp;
    q31_t xh_new;
    q31_t ap_y;
    for (i = 0; i < SAMPLE_NUM; i++){
        arm_scale_q31(&(tmp->c), tmp->xh, Q_MULT_SHIFT, &calc_tmp, 1);
        xh_new = (pData[i] - calc_tmp);

        arm_scale_q31(&(tmp->c), xh_new, Q_MULT_SHIFT, &calc_tmp, 1);
        ap_y = calc_tmp + tmp->xh;

        tmp->xh = xh_new;

        arm_scale_q31(&(tmp->H0), pData[i] + ap_y, Q_MULT_SHIFT, &calc_tmp, 1);
        pData[i] = calc_tmp >> 1;
        pData[i] += calc_tmp;
    }
    return;

}

void delete_ShelvingFilter(void *opaque){
    struct ShelvingFilter_t *tmp = (struct ShelvingFilter_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_ShelvingFilter(void *opaque, uint8_t* values){
    struct ShelvingFilter_t *tmp = (struct ShelvingFilter_t*)opaque;

    LinkPot(&(tmp->gain), values[0]);


    q31_t V0 = (q31_t)(powf(10, (tmp->gain.value * 0.1f)) * Q_1);
    tmp->H0 = V0 - Q_1;
    float Wc = 2 * 700 / (1000 * SAMPLING_RATE);

    q31_t calc_tmp = (q31_t)(Wc * Q_1) >> 2;
    calc_tmp = arm_tan_q31(calc_tmp);

    if (tmp->gain.value >= 0){
        tmp->c = ((float)((calc_tmp - Q_1) / Q_1)) / 
                 ((float)((calc_tmp + Q_1) / Q_1)) * Q_1;
        
    }else{
        tmp->c = ((float)((calc_tmp - V0) / Q_1)) / 
                 ((float)((calc_tmp + V0) / Q_1)) * Q_1;
    
    }
    // tmp->cache = (q31_t)(powf(10, (tmp->volume.value * 0.1f)) * Q_1);

    return;
}

void getParam_ShelvingFilter(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    struct ShelvingFilter_t *tmp = (struct ShelvingFilter_t*)opaque;
    *paramNum = 3;
    param[0] = &tmp->gain;
    return;
}

struct Effect_t* new_ShelvingFilter(){
    struct ShelvingFilter_t* tmp = pvPortMalloc(sizeof(struct ShelvingFilter_t));

    strcpy(tmp->parent.name, ShelvingFilterId.name);
    tmp->parent.func = ShelvingFilter;
    tmp->parent.del = delete_ShelvingFilter;
    tmp->parent.adj = adjust_ShelvingFilter;
    tmp->parent.getParam = getParam_ShelvingFilter;

    tmp->gain.name = "Gain";
    tmp->gain.upperBound = 1.0f;
    tmp->gain.lowerBound = -15.0f;
    tmp->gain.value = 1.0f;

    q31_t V0 = (q31_t)(powf(10, (tmp->gain.value * 0.1f)) * Q_1);
    tmp->H0 = V0 - Q_1;
    float Wc = 2 * 700 / (1000 * SAMPLING_RATE);
    q31_t calc_tan = arm_tan_q31((q31_t)(PI * Wc * Q_1) >> 1);
    if (tmp->gain.value >= 0){
        tmp->c = ((float)((calc_tan - Q_1) / Q_1)) / 
                 ((float)((calc_tan + Q_1) / Q_1)) * Q_1;
        
    }else{
        tmp->c = ((float)((calc_tan - V0) / Q_1)) / 
                 ((float)((calc_tan + V0) / Q_1)) * Q_1;
    
    }

    return (struct Effect_t*)tmp;
}

