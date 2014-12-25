#include "phaser.h"
#include "helper.h"

float iir_coeff_b[3] = {0.25, 0.5, 0.25};
float iir_coeff_a[2] = {1.2940575444, -0.5520629632};

q31_t coeffTable[5] = {744051710, -1164315096, 481197437, 702532174, -272930673};

arm_biquad_casd_df1_inst_q31 S;
q31_t biquadState[4];

void Phaser(q31_t* pData, void *opaque){
    struct Phaser_t *tmp = (struct Phaser_t*)opaque;
    uint32_t i;
    q31_t fData[256];

    arm_biquad_cascade_df1_q31(&S, pData, fData, SAMPLE_NUM);
    arm_copy_q31(fData, pData, SAMPLE_NUM);

    return;
}

void delete_Phaser(void *opaque){
    return;
}

void adjust_Phaser(void *opaque, uint8_t* values){
    struct Phaser_t *tmp = (struct Phaser_t*)opaque;

    LinkPot(&(tmp->speed), values[0]);  

    adjust_LFO_speed(&(tmp->lfo), tmp->speed.value);
    return;
}

struct Effect_t* new_Phaser(struct Phaser_t* opaque){
    uint32_t i;

    strcpy(opaque->parent.name, "Phaser");
    opaque->parent.func = Phaser;
    opaque->parent.del = delete_Phaser;
    opaque->parent.adj = adjust_Phaser;

    opaque->speed.upperBound = 500.0f;
    opaque->speed.lowerBound = 5000.0;
    opaque->speed.value = 2000.0f;

    new_LFO(&(opaque->lfo), 4, 0, 1000);

    coeffTable[0] = iir_coeff_b[0] * Q_1;
    coeffTable[1] = iir_coeff_b[1] * Q_1;
    coeffTable[2] = iir_coeff_b[2] * Q_1;
    coeffTable[3] = iir_coeff_a[0] * Q_1;
    coeffTable[4] = iir_coeff_a[1] * Q_1;

    arm_biquad_cascade_df1_init_q31(&S, 1, coeffTable, biquadState, Q_MULT_SHIFT);
    biquadState[0] = 0;
    biquadState[1] = 0;
    biquadState[2] = 0;
    biquadState[3] = 0;

    return (struct Effect_t*)opaque;
}

