#include "phaser.h"
#include "helper.h"

float coeff10k[16] = { -0.037367988377809525, -0.016523672267794609, -0.048529494553804398, \
                        -0.040865261107683182, -0.090225353837013245, -0.102609783411026, \ 
                        -0.22072523832321167, -0.61925923824310303,  0.61925923824310303, \
                        0.22072523832321167,  0.102609783411026,  0.090225353837013245, \
                        0.040865261107683182,  0.048529494553804398,  0.016523672267794609, \
                        0.037367988377809525};
/*
float coeff10k[6] = {
    0.00057913, -0.0015979 ,  0.0010259 ,  0.0010259 , -0.0015979 ,
    0.00057913
};
*/

void Phaser(volatile float* pData, void *opaque){
    struct Phaser_t *tmp = (struct Phaser_t*)opaque;
    uint32_t i;

    arm_fir_instance_f32 S;
    arm_status status;
    float32_t firStateF32[SAMPLE_NUM + 16 - 1];
    float32_t firOutput[SAMPLE_NUM];
    
    arm_fir_init_f32(&S, 16, (float32_t *)&coeff10k[0], &firStateF32[0], SAMPLE_NUM);

    arm_fir_f32(&S, pData , firOutput, SAMPLE_NUM);

    for(i = 0; i < SAMPLE_NUM; i++){
        *pData = *firOutput;
    }
    //Combine(pData, firOutput);
    return;
}

void delete_Phaser(void *opaque){
    return;
}

void adjust_Phaser(void *opaque, uint8_t* values){
    return;
}

struct Effect_t* new_Phaser(struct Phaser_t* opaque){
    strcpy(opaque->parent.name, "Phaser");
    opaque->parent.func = Phaser;
    opaque->parent.del = delete_Phaser;
    opaque->parent.adj = adjust_Phaser;

    return (struct Effect_t*)opaque;
}

