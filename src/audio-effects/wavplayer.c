#include "wavplayer.h"
#include "helper.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "ff.h"

void readWaveTask(void *pvParameters){
    struct WavPlayer_t *tmp = (struct WavPlayer_t*)pvParameters;

    uint8_t buffer[1024];
    uint32_t bufferIndex = 1;
    uint16_t wavDataLeft = 0;
    UINT i;

    FIL fil;

    if (f_open(&fil, "0:/tst.wav", FA_OPEN_ALWAYS | FA_READ) != FR_OK) for(;;);

    f_read(&fil, &tmp->header, sizeof(struct wavHeader_t), &i);
    if(i < sizeof(struct wavHeader_t))
        while(1);

    wavDataLeft = tmp->header.dataSize;

    while(wavDataLeft){
        if(xSemaphoreTake(tmp->Read_Hold, portMAX_DELAY)){
            f_read(&fil, buffer, SAMPLE_NUM * 2 * 2, &i);
            arm_q15_to_q31((q15_t*)buffer, tmp->dataBuffer[bufferIndex], SAMPLE_NUM * 2);

            wavDataLeft -= SAMPLE_NUM * 2 * 2;
            bufferIndex = !bufferIndex;
        }
    }

    arm_fill_q31(0, tmp->dataBuffer[0], SAMPLE_NUM);
    arm_fill_q31(0, tmp->dataBuffer[1], SAMPLE_NUM);

    while(1);
}

void WavPlayer(q31_t* pData, void *opaque){
    struct WavPlayer_t *tmp = (struct WavPlayer_t*)opaque;

    arm_scale_q31(tmp->dataBuffer[tmp->bufferIndex], tmp->cache, Q_MULT_SHIFT, tmp->dataBuffer[tmp->bufferIndex], SAMPLE_NUM);
    arm_add_q31(pData, tmp->dataBuffer[tmp->bufferIndex], pData, SAMPLE_NUM);
    tmp->bufferIndex = !tmp->bufferIndex;

    xSemaphoreGive(tmp->Read_Hold);

    return;
}

void delete_WavPlayer(void *opaque){
    return;
}

void adjust_WavPlayer(void *opaque, uint8_t* values){
    struct WavPlayer_t *tmp = (struct WavPlayer_t*)opaque;

    LinkPot(&(tmp->volume), values[0]);

    tmp->cache = (q31_t)(powf(10, (tmp->volume.value * 0.1f)) * Q_1);

    return;
}

void getParam_WavPlayer(void *opaque, struct parameter_t param[], uint8_t* paramNum){
    struct WavPlayer_t *tmp = (struct WavPlayer_t*)opaque;
    *paramNum = 1;
    param[0].value = tmp->volume.value;
    return;
}

struct Effect_t* new_WavPlayer(struct WavPlayer_t* opaque){

    strcpy(opaque->parent.name, "WavPlayer");
    opaque->parent.func = WavPlayer;
    opaque->parent.del = delete_WavPlayer;
    opaque->parent.adj = adjust_WavPlayer;
    opaque->parent.getParam = getParam_WavPlayer;

    opaque->volume.upperBound = 0.0f;
    opaque->volume.lowerBound = -20.0;
    opaque->volume.value = 0.0f;

    opaque->cache = (q31_t)(powf(10, (opaque->volume.value * 0.1f)) * Q_1);

    opaque->bufferIndex = 0;
    
    opaque->Read_Hold = xSemaphoreCreateBinary();
    xSemaphoreGive(opaque->Read_Hold);

	xTaskCreate(readWaveTask,
	            (signed char*)"RWT",
	            2048, NULL, tskIDLE_PRIORITY + 1, NULL);

    return (struct Effect_t*)opaque;
}

