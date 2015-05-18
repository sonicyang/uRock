#include "peakFilter.h"
#include "helper.h"
#include "FreeRTOS.h"

void PeakFilter(q31_t* pData, void *opaque){
    struct PeakFilter_t *tmp = (struct PeakFilter_t*)opaque;
    register uint32_t i;
    /*q31_t V0 = opaque->gain.value; // pow10(opaque->G.value * 0.05 * Q_1) ;*/
    /*q31_t H0 = V0 - 1;*/

    q31_t calc_tmp;
    q31_t xh_new;
    q31_t ap_y;
    /*if (G >= 0){*/
    /*    opaque->c = (np.tan((q31_t)(np.pi * Wb * Q_1) >> 1) - 1) / */
    /*        (np.tan((q31_t)(np.pi * Wb * Q_1) >> 1) + 1);*/
        
    /*}else{*/
    /*    opaque->c = (np.tan(np.pi * Wb * 0.5) - V0) /*/
    /*        (np.tan(np.pi * Wb * 0.5) + V0) * Q_1;*/
    
    /*}*/
    /*opaque->d = -cos(PI * Wc * Q_1);*/
    for (i = 0; i < SAMPLE_NUM; i++){
        arm_scale_q31(&(tmp->d), (Q_1 - tmp->c), Q_MULT_SHIFT, &calc_tmp, 1);
        arm_scale_q31(&calc_tmp, tmp->xh0, Q_MULT_SHIFT, &calc_tmp, 1);
        xh_new = -calc_tmp;
        arm_scale_q31(&(tmp->c), tmp->xh1, Q_MULT_SHIFT, &calc_tmp, 1);
        xh_new += (pData[i] + calc_tmp);

        arm_scale_q31(&(tmp->c), -xh_new, Q_MULT_SHIFT, &calc_tmp, 1);
        ap_y = calc_tmp;
        arm_scale_q31(&(tmp->d), (Q_1 - tmp->c), Q_MULT_SHIFT, &calc_tmp, 1);
        arm_scale_q31(&calc_tmp, tmp->xh0, Q_MULT_SHIFT, &calc_tmp, 1);
        ap_y += (calc_tmp + tmp->xh1);

        tmp->xh0 = xh_new;
        tmp->xh1 = tmp->xh0;

        arm_scale_q31(&(tmp->H0), pData[i] - ap_y, Q_MULT_SHIFT, &calc_tmp, 1);
        pData[i] += calc_tmp >> 1;
    }
    return;

}

void delete_PeakFilter(void *opaque){
    struct PeakFilter_t *tmp = (struct PeakFilter_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_PeakFilter(void *opaque, uint8_t* values){
    struct PeakFilter_t *tmp = (struct PeakFilter_t*)opaque;

    LinkPot(&(tmp->gain), values[0]);


    q31_t V0 = (q31_t)(powf(10, (tmp->gain.value * 0.1f)) * Q_1);
    tmp->H0 = V0 - Q_1;
    float Wb = 2 * 20 / (1000 * SAMPLING_RATE);
    float Wc = 2 * 2000 / (1000 * SAMPLING_RATE);
    q31_t calc_tmp = (q31_t)(Wb * Q_1) >> 2;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;

    calc_tmp = arm_tan_q31(calc_tmp);
    if (tmp->gain.value >= 0){
        tmp->c = ((float)((calc_tmp - Q_1) / Q_1)) / 
                 ((float)((calc_tmp + Q_1) / Q_1)) * Q_1;
        
    }else{
        tmp->c = ((float)((calc_tmp - V0) / Q_1)) / 
                 ((float)((calc_tmp + V0) / Q_1)) * Q_1;
    
    }
    calc_tmp = (q31_t)(Wc * Q_1) >> 1;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;
    tmp->d = -arm_cos_q31(calc_tmp);
    // tmp->cache = (q31_t)(powf(10, (tmp->volume.value * 0.1f)) * Q_1);

    return;
}

void getParam_PeakFilter(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    struct PeakFilter_t *tmp = (struct PeakFilter_t*)opaque;
    *paramNum = 1;
    param[0] = &tmp->gain;
    return;
}

struct Effect_t* new_PeakFilter(){
    struct PeakFilter_t* tmp = pvPortMalloc(sizeof(struct PeakFilter_t));

    strcpy(tmp->parent.name, PeakFilterId.name);
    tmp->parent.func = PeakFilter;
    tmp->parent.del = delete_PeakFilter;
    tmp->parent.adj = adjust_PeakFilter;
    tmp->parent.getParam = getParam_PeakFilter;

    tmp->gain.name = "Gain";
    tmp->gain.upperBound = 1.0f;
    tmp->gain.lowerBound = -15.0f;
    tmp->gain.value = 1.0f;

    q31_t V0 = (q31_t)(powf(10, (tmp->gain.value * 0.1f)) * Q_1);
    tmp->H0 = V0 - Q_1;
    float Wb = 2 * 20 / (1000 * SAMPLING_RATE);
    float Wc = 2 * 2000 / (1000 * SAMPLING_RATE);
    q31_t calc_tmp = (q31_t)(Wb * Q_1) >> 2;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;

    calc_tmp = arm_tan_q31(calc_tmp);
    if (tmp->gain.value >= 0){
        tmp->c = ((float)((calc_tmp - Q_1) / Q_1)) / 
                 ((float)((calc_tmp + Q_1) / Q_1)) * Q_1;
        
    }else{
        tmp->c = ((float)((calc_tmp - V0) / Q_1)) / 
                 ((float)((calc_tmp + V0) / Q_1)) * Q_1;
    
    }
    calc_tmp = (q31_t)(Wc * Q_1) >> 1;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;
    tmp->d = -arm_cos_q31(calc_tmp);
    return (struct Effect_t*)tmp;
}

