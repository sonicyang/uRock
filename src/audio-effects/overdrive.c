#include "overdrive.h"
#include "helper.h"

void Overdrive(q31_t* pData, void *opaque){
    struct Overdrive_t *tmp = (struct Overdrive_t*)opaque;
    register uint32_t i;
    register float gg = SAMPLE_MAX;
    q31_t q31_ratio = tmp->ratio.value * Q_1;
    q31_t q31_r_ratio = Q_1 / tmp->ratio.value;
    q31_t q31_gg = gg * Q_1;
    q31_t q31_r_gg = 2147483648 / gg; //this is fking Q1.31
    q31_t q31_rg;
    q31_t q31_r_rg;

    arm_scale_q31(&q31_gg, q31_ratio, Q_MULT_SHIFT, &q31_rg, 1);
    arm_scale_q31(&q31_r_gg, q31_r_ratio, Q_MULT_SHIFT, &q31_r_rg, 1);

    for (i = 0; i < SAMPLE_NUM; i++){

        if (pData[i] > q31_rg){
            pData[i] = q31_gg;
        }else if(pData[i] < -q31_rg){
            pData[i] = -q31_gg;
        }else{

            q31_t tmp;
            arm_scale_q31(pData + i, q31_r_rg, 0, &tmp, 1);
            arm_scale_q31(&tmp, 0.25 * Q_1, Q_MULT_SHIFT * 2, &tmp, 1);
            tmp = arm_sin_q31(tmp);
            arm_shift_q31(&tmp, -Q_MULT_SHIFT, &tmp, 1);
            arm_scale_q31(&tmp, q31_gg, Q_MULT_SHIFT, &pData[i], 1);
            //pData[i] = arm_sin_f32(PI * pData[i] * 0.5 * q31_r_rg) * q31_gg;
            //pData[i] = sin(pData[i] * 0.25 * q31_r_rg) * q_31_gg ;
        }
    }

    return;
}

void delete_Overdrive(void *opaque){
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

struct Effect_t* new_Overdrive(struct Overdrive_t* opaque){
    strcpy(opaque->parent.name, "Overdrive");
    opaque->parent.func = Overdrive;
    opaque->parent.del = delete_Overdrive;
    opaque->parent.adj = adjust_Overdrive;

    opaque->gain.upperBound = 15.0f;
    opaque->gain.lowerBound = 0.0f;
    opaque->gain.value = 0.0f;

    opaque->volume.upperBound = 0.0f;
    opaque->volume.lowerBound = -30.0f;
    opaque->volume.value = 0.0f;

    opaque->ratio.upperBound = 1.0f;
    opaque->ratio.lowerBound = 0.5f;
    opaque->ratio.value = 1.0f;

    return (struct Effect_t*)opaque;
}

