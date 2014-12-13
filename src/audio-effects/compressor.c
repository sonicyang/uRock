#include "compressor.h"
#include "helper.h"

void Compressor(volatile float* pData, void *opaque){
    struct Compressor_t *tmp = (struct Compressor_t*)opaque;
    static uint8_t pre_status = 0;
    static uint32_t count = 0;
    static float pre_r;
    float rRatio = 1.0f / tmp->ratio.value;
    float volume = 0.0f;
    float dbvolume;
    float dbthreshold;
    float rate = 1.0f;
    uint8_t status = 0;
    float aData;
    float attack_block_count = tmp->attack.value / BLOCK_PREIOD;

    for (int i = 0; i < SAMPLE_NUM; i++){
        if (pData[i] < 0.0f)
            aData = 0.0f - pData[i];
        if (aData > volume){
            volume = aData;
            if (aData > tmp->threshold.value)
                status = 1;
        }
    }
    dbvolume = logf(volume / 127.0f);
    dbthreshold = logf(tmp->threshold.value / 127.0f);
    if (pre_status != status){
        count = 0;
        pre_status = status;
    }
    if (count <= attack_block_count){
        rate = count / attack_block_count;
    }
    float temp;
    if (status == 1){
        pre_r = dbvolume - dbthreshold;
        temp = powf(10, -pre_r * rRatio * rate);
    }else{
        temp = powf(10, -pre_r * rRatio + pre_r * rRatio * rate);
    }
    for (int i = 0; i < SAMPLE_NUM; i++){
        pData[i] = pData[i] * temp;
    }
    count ++;
    
    
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
