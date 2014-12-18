#include "phaser.h"
#include "helper.h"

float32_t iir_coeff_b[3] = { 0.95466064, -1.93563801,  1.0};
float32_t iir_coeff_a[2] = { -1.93563801,  0.95466064};

float32_t iir_chain_coeff[20] = {0.87302845, -1.71109663,  1.0, -1.71109663,  0.87302845, 0.87302845, -1.71109663,  1.0, -1.71109663,  0.87302845, 0.87302845, -1.71109663,  1.0, -1.71109663,  0.87302845, 0.87302845, -1.71109663,  1.0, -1.71109663,  0.87302845};

arm_biquad_cascade_df2T_instance_f32 S[4];
float32_t iirStateF32[4][8];
float32_t iirOutput[SAMPLE_NUM];

void Phaser(volatile float* pData, void *opaque){
    struct Phaser_t *tmp = (struct Phaser_t*)opaque;
    uint32_t i;
    float fData[256];

    Copy(fData, pData);
    //arm_biquad_cascade_df2T_f32(&S[(uint32_t)tmp->lfo.next(&tmp->lfo)], pData, iirOutput, SAMPLE_NUM);
    arm_biquad_cascade_df2T_f32(&S[3], fData, iirOutput, SAMPLE_NUM);
    
    for(i = 0; i < SAMPLE_NUM; i++){
        pData[i] = iirOutput[i];
    }
    
    //Gain(iirOutput, -20);
    //Combine(pData, iirOutput);
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
/*
    for(i = 0; i < 20; i += 5){
        iir_chain_coeff[i] = iir_coeff_b[0];
        iir_chain_coeff[i + 1] = iir_coeff_b[1];
        iir_chain_coeff[i + 2] = iir_coeff_b[2];
        iir_chain_coeff[i + 3] = iir_coeff_a[0];
        iir_chain_coeff[i + 4] = iir_coeff_a[1];
    }
*/
    arm_biquad_cascade_df2T_init_f32(&S[0], 1, iir_chain_coeff, iirStateF32[0]); 
    arm_biquad_cascade_df2T_init_f32(&S[1], 2, iir_chain_coeff, iirStateF32[1]); 
    arm_biquad_cascade_df2T_init_f32(&S[2], 3, iir_chain_coeff, iirStateF32[2]); 
    arm_biquad_cascade_df2T_init_f32(&S[3], 4, iir_chain_coeff, iirStateF32[3]); 


    return (struct Effect_t*)opaque;
}

