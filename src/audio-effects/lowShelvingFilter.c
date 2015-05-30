#include "lowShelvingFilter.h"
#include "helper.h"
#include "FreeRTOS.h"

/*q31_t arm_tan_q31(q31_t x){
    return ((float)(arm_sin_q31(x) / Q_1)) /
           ((float)(arm_cos_q31(x) / Q_1)) * Q_1;

}*/
void LowShelvingFilter(q31_t* pData, void *opaque){
    struct LowShelvingFilter_t *tmp = (struct LowShelvingFilter_t*)opaque;
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
        calc_tmp = calc_tmp >> 1;
        pData[i] += calc_tmp;
    }
    return;

}

void delete_LowShelvingFilter(void *opaque){
    struct LowShelvingFilter_t *tmp = (struct LowShelvingFilter_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_LowShelvingFilter(void *opaque, uint8_t* values){
    struct LowShelvingFilter_t *tmp = (struct LowShelvingFilter_t*)opaque;

    LinkPot(&(tmp->gain), values[0]);


    q31_t V0 = (q31_t)(powf(10, (tmp->gain.value * 0.05f)) * Q_1);
    tmp->H0 = V0 - Q_1;
    float Wc = 2. * 50. / (1000. * SAMPLING_RATE);

    q31_t calc_tmp = (q31_t)(Wc * Q_1) >> 2;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;
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

void getParam_LowShelvingFilter(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    struct LowShelvingFilter_t *tmp = (struct LowShelvingFilter_t*)opaque;
    *paramNum = 1;
    param[0] = &tmp->gain;
    return;
}

struct Effect_t* new_LowShelvingFilter(){
    struct LowShelvingFilter_t* tmp = pvPortMalloc(sizeof(struct LowShelvingFilter_t));

    strcpy(tmp->parent.name, LowShelvingFilterId.name);
    tmp->parent.func = LowShelvingFilter;
    tmp->parent.del = delete_LowShelvingFilter;
    tmp->parent.adj = adjust_LowShelvingFilter;
    tmp->parent.getParam = getParam_LowShelvingFilter;

    tmp->gain.name = "Gain";
    tmp->gain.upperBound = 0.0f;
    tmp->gain.lowerBound = -15.0f;
    tmp->gain.value = 0.0f;

    tmp->xh = 0;
    q31_t V0 = (q31_t)(powf(10, (tmp->gain.value * 0.05f)) * Q_1);
    tmp->H0 = V0 - Q_1;
    float Wc = 2. * 50. / (1000. * SAMPLING_RATE);

    q31_t calc_tmp = (q31_t)(Wc * Q_1) >> 2;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;
    calc_tmp = arm_tan_q31(calc_tmp);


    if (tmp->gain.value >= 0){
        tmp->c = ((float)((calc_tmp - Q_1) / Q_1)) / 
                 ((float)((calc_tmp + Q_1) / Q_1)) * Q_1;
        
    }else{
        tmp->c = ((float)((calc_tmp - V0) / Q_1)) / 
                 ((float)((calc_tmp + V0) / Q_1)) * Q_1;
    
    }

    return (struct Effect_t*)tmp;
}

