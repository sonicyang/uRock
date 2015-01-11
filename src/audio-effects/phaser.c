#include "phaser.h"
#include "helper.h"
#include "FreeRTOS.h"

float iir_coeff_b[3] = {0.25, 0.5, 0.25};
float iir_coeff_a[2] = {1.2940575444, -0.5520629632};

//float iir_coeff[10] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0};
float iir_coeff[10] = {1.35741138584944e-06, 2.71482285494618e-06, 1.35741140509988e-06, 1.87433370644654, -0.878910638328047, 1.99999993867203, 0.999999985818274, 1.94325363119022, 0};

arm_biquad_casd_df1_inst_q31 S;
q31_t biquadState[8];

void Phaser(q31_t* pData, void *opaque){
    q31_t fData[256];

    arm_biquad_cascade_df1_q31(&S, pData, fData, SAMPLE_NUM);
    arm_copy_q31(fData, pData, SAMPLE_NUM);

    return;
}

void delete_Phaser(void *opaque){
    struct Phaser_t *tmp = (struct Phaser_t*)opaque;
    vPortFree(tmp); 
    return;
}

void adjust_Phaser(void *opaque, uint8_t* values){
    struct Phaser_t *tmp = (struct Phaser_t*)opaque;

    LinkPot(&(tmp->speed), values[0]);  

    adjust_LFO_speed(&(tmp->lfo), tmp->speed.value);
    return;
}

void getParam_Phaser(void *opaque, struct parameter_t param[], uint8_t* paramNum){
    struct Phaser_t *tmp = (struct Phaser_t*)opaque;
    *paramNum = 1;
    param[0].value = tmp->speed.value;
    return;
}

struct Effect_t* new_Phaser(){
    struct Phaser_t* tmp = pvPortMalloc(sizeof(struct Phaser_t));

    strcpy(tmp->parent.name, "Phaser");
    tmp->parent.func = Phaser;
    tmp->parent.del = delete_Phaser;
    tmp->parent.adj = adjust_Phaser;
    tmp->parent.getParam = getParam_Phaser;

    tmp->speed.upperBound = 500.0f;
    tmp->speed.lowerBound = 5000.0;
    tmp->speed.value = 2000.0f;

    new_LFO(&(tmp->lfo), 4, 0, 1000);


    return (struct Effect_t*)tmp;
}

