#include "equalizer.h"
#include "helper.h"
#include "FreeRTOS.h"


void adjust_function(struct Equalizer_t* tmp){
    // high shelving filter
    float Wb, Wc;
    q31_t V0, calc_tmp;

    V0 = (q31_t)(powf(10, (tmp->high.value * 0.05f)) * Q_1);
    tmp->h_H0 = V0 - Q_1;
    Wc = 2. * 2100. / (1000. * SAMPLING_RATE);

    calc_tmp = (q31_t)(Wc * Q_1) >> 2;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;
    calc_tmp = arm_tan_q31(calc_tmp);

    if (tmp->high.value >= 0){
        tmp->h_c = ((float)((calc_tmp - Q_1) / Q_1)) / 
                 ((float)((calc_tmp + Q_1) / Q_1)) * Q_1;
    }else{
        tmp->h_c = ((float)((calc_tmp - V0) / Q_1)) / 
                 ((float)((calc_tmp + V0) / Q_1)) * Q_1;
    }

    //peak filter
    V0 = (q31_t)(powf(10, (tmp->mid.value * 0.1f)) * Q_1);
    tmp->m_H0 = V0 - Q_1;
    Wb = 2. * 20. / (1000. * SAMPLING_RATE);
    Wc = 2. * 700. / (1000. * SAMPLING_RATE);
    calc_tmp = (q31_t)(Wb * Q_1) >> 2;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;

    calc_tmp = arm_tan_q31(calc_tmp);
    if (tmp->mid.value >= 0){
        tmp->m_c = ((float)((calc_tmp - Q_1) / Q_1)) / 
                 ((float)((calc_tmp + Q_1) / Q_1)) * Q_1;
    }else{
        tmp->m_c = ((float)((calc_tmp - V0) / Q_1)) / 
                 ((float)((calc_tmp + V0) / Q_1)) * Q_1;
    }
    calc_tmp = (q31_t)(Wc * Q_1) >> 1;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;
    tmp->m_d = -arm_cos_q31(calc_tmp);
    
    //low shelving filter
    V0 = (q31_t)(powf(10, (tmp->low.value * 0.05f)) * Q_1);
    tmp->l_H0 = V0 - Q_1;
    Wc = 2. * 50. / (1000. * SAMPLING_RATE);

    calc_tmp = (q31_t)(Wc * Q_1) >> 2;
    calc_tmp = calc_tmp << Q_MULT_SHIFT;
    calc_tmp = arm_tan_q31(calc_tmp);

    if (tmp->low.value >= 0){
        tmp->l_c = ((float)((calc_tmp - Q_1) / Q_1)) / 
                 ((float)((calc_tmp + Q_1) / Q_1)) * Q_1;
    }else{
        tmp->l_c = ((float)((calc_tmp - V0) / Q_1)) / 
                 ((float)((calc_tmp + V0) / Q_1)) * Q_1;
    }
}

void Equalizer(q31_t* pData, void *opaque){
    struct Equalizer_t *tmp = (struct Equalizer_t*)opaque;
    register uint32_t i;

    q31_t calc_tmp;
    q31_t xh_new;
    q31_t ap_y;
    // high shelving filter
    for (i = 0; i < SAMPLE_NUM; i++){
        arm_scale_q31(&(tmp->h_c), tmp->h_xh, Q_MULT_SHIFT, &calc_tmp, 1);
        xh_new = (pData[i] - calc_tmp);

        arm_scale_q31(&(tmp->h_c), xh_new, Q_MULT_SHIFT, &calc_tmp, 1);
        ap_y = calc_tmp + tmp->h_xh;

        tmp->h_xh = xh_new;

        arm_scale_q31(&(tmp->h_H0), pData[i] - ap_y, Q_MULT_SHIFT, &calc_tmp, 1);
        calc_tmp = calc_tmp >> 1;
        pData[i] += calc_tmp;
    }

    // peak filter
    for (i = 0; i < SAMPLE_NUM; i++){
        arm_scale_q31(&(tmp->m_d), (Q_1 - tmp->m_c), Q_MULT_SHIFT, &calc_tmp, 1);
        arm_scale_q31(&calc_tmp, tmp->m_xh0, Q_MULT_SHIFT, &calc_tmp, 1);
        xh_new = -calc_tmp;
        arm_scale_q31(&(tmp->m_c), tmp->m_xh1, Q_MULT_SHIFT, &calc_tmp, 1);
        xh_new += (pData[i] + calc_tmp);

        arm_scale_q31(&(tmp->m_c), -xh_new, Q_MULT_SHIFT, &calc_tmp, 1);
        ap_y = calc_tmp;
        arm_scale_q31(&(tmp->m_d), (Q_1 - tmp->m_c), Q_MULT_SHIFT, &calc_tmp, 1);
        arm_scale_q31(&calc_tmp, tmp->m_xh0, Q_MULT_SHIFT, &calc_tmp, 1);
        ap_y += (calc_tmp + tmp->m_xh1);

        tmp->m_xh0 = xh_new;
        tmp->m_xh1 = tmp->m_xh0;

        arm_scale_q31(&(tmp->m_H0), pData[i] - ap_y, Q_MULT_SHIFT, &calc_tmp, 1);
        pData[i] += calc_tmp >> 1;
    }

    //low shelving filter
    for (i = 0; i < SAMPLE_NUM; i++){
        arm_scale_q31(&(tmp->l_c), tmp->l_xh, Q_MULT_SHIFT, &calc_tmp, 1);
        xh_new = (pData[i] - calc_tmp);

        arm_scale_q31(&(tmp->l_c), xh_new, Q_MULT_SHIFT, &calc_tmp, 1);
        ap_y = calc_tmp + tmp->l_xh;

        tmp->l_xh = xh_new;

        arm_scale_q31(&(tmp->l_H0), pData[i] + ap_y, Q_MULT_SHIFT, &calc_tmp, 1);
        calc_tmp = calc_tmp >> 1;
        pData[i] += calc_tmp;
    }
    return;

}

void delete_Equalizer(void *opaque){
    struct Equalizer_t *tmp = (struct Equalizer_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_Equalizer(void *opaque, uint8_t* values){
    struct Equalizer_t *tmp = (struct Equalizer_t*)opaque;

    LinkPot(&(tmp->high), values[0]);
    LinkPot(&(tmp->mid), values[1]);
    LinkPot(&(tmp->low), values[2]);

    // high shelving filter
    adjust_function(tmp);

    return;
}

void getParam_Equalizer(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    struct Equalizer_t *tmp = (struct Equalizer_t*)opaque;
    *paramNum = 3;
    param[0] = &tmp->high;
    param[1] = &tmp->mid;
    param[2] = &tmp->low;
    return;
}

struct Effect_t* new_Equalizer(){
    struct Equalizer_t* tmp = pvPortMalloc(sizeof(struct Equalizer_t));

    strcpy(tmp->parent.name, EqualizerId.name);
    tmp->parent.func = Equalizer;
    tmp->parent.del = delete_Equalizer;
    tmp->parent.adj = adjust_Equalizer;
    tmp->parent.getParam = getParam_Equalizer;

    tmp->high.name = "High";
    tmp->high.upperBound = 0.0f;
    tmp->high.lowerBound = -15.0f;
    tmp->high.value = 0.0f;

    tmp->mid.name = "Mid";
    tmp->mid.upperBound = 0.0f;
    tmp->mid.lowerBound = -15.0f;
    tmp->mid.value = 0.0f;

    tmp->low.name = "Low";
    tmp->low.upperBound = 0.0f;
    tmp->low.lowerBound = -15.0f;
    tmp->low.value = 0.0f;

    tmp->h_xh = 0;

    tmp->m_xh0 = 0;
    tmp->m_xh1 = 0;

    tmp->l_xh = 0;

    adjust_function(tmp);
    return (struct Effect_t*)tmp;
}
