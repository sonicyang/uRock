#include "overdrive.h"
#include "helper.h"

void Overdrive(q31_t* pData, void *opaque){
    struct Overdrive_t *tmp = (struct Overdrive_t*)opaque;
    uint32_t i;
    float rg = tmp->ratio.value * SAMPLE_MAX;
    float r_rg = 1 / rg;
    q31_t q31_rg = rg * Q_1;
    float floating;

    arm_scale_q31(pData, tmp->gain.value * Q_1, Q_MULT_SHIFT, pData, SAMPLE_NUM);


    for (i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] > q31_rg){
            pData[i] = q31_rg;
        }
        else if(pData[i] < -q31_rg){
            pData[i] = -q31_rg;
        }
        else{
            floating = pData[i] / Q_1;
            floating = arm_sin_f32(PI * floating * 0.5 * r_rg) * SAMPLE_MAX;
            pData[i] = floating * Q_1;
        }
    }

    arm_scale_q31(pData, tmp->cache, Q_MULT_SHIFT, pData, SAMPLE_NUM);
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

