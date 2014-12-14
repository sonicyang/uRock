#include "compressor.h"
#include "helper.h"

void Compressor(volatile float* pData, void *opaque){
    struct Compressor_t *tmp = (struct Compressor_t*)opaque;
    float rRatio = 1.0f / tmp->ratio.value;
    float volume = 0.0f;
    float dbvolume = -30.0f;
    float gg = powf(10, (tmp->threshold.value * 0.1f)) * SAMPLE_MAX;
    float rate = 1.0f;
    uint8_t status = 0;
    float aData;
    float attack_block_count = tmp->attack.value / BLOCK_PERIOD;
    for (int i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] < 0.0f)
            aData = 0.0f - pData[i];
        if (aData > volume){
            volume = aData;
            if (aData > gg)
                status = 1;
        }
    }
    if (volume != 0)
        dbvolume = 10.0f * logf(volume / SAMPLE_MAX);
    if (tmp->pre_status != status){
        tmp->count = 0;
        tmp->pre_status = status;
    }
    if (tmp->count <= attack_block_count){
        rate = tmp->count / attack_block_count;
    }
    float temp;
    if (status == 1){
        tmp->pre_r = dbvolume - tmp->threshold.value;
        temp = powf(10, -tmp->pre_r * rRatio * rate);
    }else{
        temp = powf(10, -tmp->pre_r * rRatio + tmp->pre_r * rRatio * rate);
    }
    for (int i = 0; i < SAMPLE_NUM; i++){
        pData[i] = pData[i] * temp;
    }
    tmp->count ++;
    
    
    return;
}

void delete_Compressor(void *opaque){
    return;
}

void adjust_Compressor(void *opaque, uint8_t* values){
    struct Compressor_t *tmp = (struct Compressor_t*)opaque;
    
    LinkPot(&(tmp->threshold), values[0]);  
    LinkPot(&(tmp->attack), values[1]);  
    LinkPot(&(tmp->ratio), values[2]);  
    
    return;
}

struct Effect_t* new_Compressor(struct Compressor_t* opaque){
    strcpy(opaque->parent.name, "Compressor");
    opaque->parent.func = Compressor;
    opaque->parent.del = delete_Compressor;
    opaque->parent.adj = adjust_Compressor;

    opaque->threshold.upperBound = 0.0f;
    opaque->threshold.lowerBound = -30.0f;
    opaque->threshold.value = 0.0f;

    opaque->attack.upperBound = 50.0f;
    opaque->attack.lowerBound = 1.0f;
    opaque->attack.value = 1.0f;
    
    opaque->ratio.upperBound = 5.0f;
    opaque->ratio.lowerBound = 1.0f;
    opaque->ratio.value = 1.0f;
    
    opaque->pre_status = 0;
    opaque->count = 0;

    return (struct Effect_t*)opaque;
}
