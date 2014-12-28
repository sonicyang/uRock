#include "phaser.h"
#include "helper.h"

float iir_coeff_b[3] = {0.25, 0.5, 0.25};
float iir_coeff_a[2] = {1.2940575444, -0.5520629632};

//float iir_coeff[10] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0};
float iir_coeff[10] = {1.35741138584944e-06, 2.71482285494618e-06, 1.35741140509988e-06, 1.87433370644654, -0.878910638328047, 1.99999993867203, 0.999999985818274, 1.94325363119022, 0};

q31_t coeffTable[10];

arm_biquad_casd_df1_inst_q31 S;
q31_t biquadState[8];

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

    for (i = 0; i < 10; i++) {
        coeffTable[i] = iir_coeff[i] * 536870912;
    }
    arm_biquad_cascade_df1_init_q31(&S, 2, coeffTable, biquadState, 2);

    return (struct Effect_t*)opaque;
}

