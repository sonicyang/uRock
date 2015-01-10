#include "equalizer.h"
#include "helper.h"

float low_iir_coeff[20] = { 3.99551947083410e-10,   7.99348161821938e-10,   3.99671234609408e-10,   1.74266431038426,   -0.759790473239173, \
                            1,                      2.02501937393162,       1.02534004997240,       1.77512221511265,   -0.792567352260218, \
                            1,                      1.99938921206706,       0.999702296645625,      1.83839093567522,   -0.856457857468755, \
                            1,                      1.97498006006623,       0.975285456788754,      1.92818340603766,   -0.947132768618419 \
                          }; 

float mid_iir_coeff[20] = { 1.35740892917766e-06, 2.71481784796122e-06, 1.35740891350065e-06, 1.91650171764863, -0.928995687002044, \
                            1                   , -1.99999999488179   , 0.999999991001735,    1.93895852529203, -0.946088677929243, \
                            1                   , 2.00000000765730    , 1.00000001154921,     1.94730193841209, -0.965578012422153, \
                            1                   , -2.00000000511822   , 1.00000000899827,     1.97673865659616, -0.981792307404491  \
                          };

float high_iir_coeff[20] = { 0.698913519775096, -1.39825432307736, 0.699122182663448, 1.74266436392243, -0.759790517615424, \
                             1,                 -2.02501937392804, 1.02534004996864,  1.77512214469618, -0.792567289923241, \
                             1,                 -1.97498006006975, 0.975285456792233, 1.83839095043520, -0.856457873475020, \
                             1,                 -1.99938921207203, 0.999702296650512, 1.92818340815596, -0.947132770093246  \
                           };


void Equalizer(q31_t* pData, void *opaque){
    struct Equalizer_t *tmp = (struct Equalizer_t*)opaque;
    q31_t fData[256];
    q31_t oData[256];

    arm_copy_q31(pData, oData, SAMPLE_NUM);
    arm_scale_q31(oData, 0.707 * Q_1, Q_MULT_SHIFT, oData, SAMPLE_NUM); // Magic!

    arm_biquad_cascade_df1_fast_q31(&tmp->S[0], oData, fData, SAMPLE_NUM);
    arm_scale_q31(fData, tmp->cache[0], Q_MULT_SHIFT, pData, SAMPLE_NUM);

    arm_biquad_cascade_df1_fast_q31(&tmp->S[1], oData, fData, SAMPLE_NUM);
    arm_scale_q31(fData, tmp->cache[1], Q_MULT_SHIFT, fData, SAMPLE_NUM);
    arm_add_q31(fData, pData, pData, SAMPLE_NUM);

    arm_biquad_cascade_df1_fast_q31(&tmp->S[2], oData, fData, SAMPLE_NUM);
    arm_scale_q31(fData, tmp->cache[2], Q_MULT_SHIFT, fData, SAMPLE_NUM);
    arm_add_q31(fData, pData, pData, SAMPLE_NUM);

    return;
}

void delete_Equalizer(void *opaque){
    return;
}

void adjust_Equalizer(void *opaque, uint8_t* values){
    struct Equalizer_t *tmp = (struct Equalizer_t*)opaque;

    LinkPot(&(tmp->low), values[0]);
    LinkPot(&(tmp->mid), values[1]);
    LinkPot(&(tmp->high), values[2]);

    tmp->cache[0] = (q31_t)(powf(10, (tmp->low.value * 0.1f)) * Q_1);
    tmp->cache[1] = (q31_t)(powf(10, (tmp->mid.value * 0.1f)) * Q_1);
    tmp->cache[2] = (q31_t)(powf(10, (tmp->high.value * 0.1f)) * Q_1);

    return;
}

void getParam_Equalizer(void *opaque, struct parameter_t param[], uint8_t* paramNum){
    struct Equalizer_t *tmp = (struct Equalizer_t*)opaque;
    *paramNum = 3;
    param[0].value = tmp->low.value;
    param[1].value = tmp->mid.value;
    param[2].value = tmp->high.value;
    return;
}

struct Effect_t* new_Equalizer(struct Equalizer_t* opaque){
    uint32_t i;

    strcpy(opaque->parent.name, "Equalizer");
    opaque->parent.func = Equalizer;
    opaque->parent.del = delete_Equalizer;
    opaque->parent.adj = adjust_Equalizer;
    opaque->parent.getParam = getParam_Equalizer;

    opaque->low.upperBound = 0.0f;
    opaque->low.lowerBound = -20.0;
    opaque->low.value = 0.0f;

    opaque->mid.upperBound = 0.0f;
    opaque->mid.lowerBound = -20.0;
    opaque->mid.value = 0.0f;

    opaque->high.upperBound = 0.0f;
    opaque->high.lowerBound = -20.0;
    opaque->high.value = 0.0f;
    opaque->cache[0] = (q31_t)(powf(10, (opaque->low.value * 0.1f)) * Q_1);
    opaque->cache[1] = (q31_t)(powf(10, (opaque->mid.value * 0.1f)) * Q_1);
    opaque->cache[2] = (q31_t)(powf(10, (opaque->high.value * 0.1f)) * Q_1);

    //TODO: Clear this 
    for (i = 0; i < 20; i++) {
        opaque->coeffTable[0][i] = low_iir_coeff[i] * 536870912;
    }

    for (i = 0; i < 20; i++) {
        opaque->coeffTable[1][i] = mid_iir_coeff[i] * 536870912;
    }

    for (i = 0; i < 20; i++) {
        opaque->coeffTable[2][i] = high_iir_coeff[i] * 536870912;
    }
    arm_biquad_cascade_df1_init_q31(&opaque->S[0], 4, opaque->coeffTable[0], opaque->biquadState[0], 2);
    arm_biquad_cascade_df1_init_q31(&opaque->S[1], 4, opaque->coeffTable[1], opaque->biquadState[1], 2);
    arm_biquad_cascade_df1_init_q31(&opaque->S[2], 4, opaque->coeffTable[2], opaque->biquadState[2], 2);

    return (struct Effect_t*)opaque;
}

