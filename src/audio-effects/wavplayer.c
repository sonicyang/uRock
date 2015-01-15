#include "wavplayer.h"
#include "helper.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "ff.h"

void readWaveTask(void *pvParameters){
    struct WavPlayer_t *tmp = (struct WavPlayer_t*)pvParameters;

    uint16_t buffer[SAMPLE_NUM * 2];
    uint32_t bufferIndex = 1;
    uint16_t wavDataLeft = 0;
    UINT i, j;

    FIL fil;

    if (f_open(&fil, tmp->filename, FA_OPEN_ALWAYS | FA_READ) != FR_OK) for(;;);

    f_read(&fil, &tmp->header, sizeof(struct wavHeader_t), &i);
    if(i < sizeof(struct wavHeader_t)) //Check Read a File
        vTaskDelete(NULL);

    if((strncmp(tmp->header.filID, "RIFF", 4) != 0) ||\
       (strncmp(tmp->header.waveID, "WAVE", 4) != 0))
        vTaskDelete(NULL);

    if((tmp->header.nChannels > 2) ||\
       (tmp->header.wFormatTag != 0x0001) ||\
       (tmp->header.nSamplesPerSec != 44100) ||\
       (tmp->header.wBitsPerSample != 16))
        vTaskDelete(NULL);

    do{
        if(xSemaphoreTake(tmp->Read_Hold, portMAX_DELAY)){
            f_read(&fil, buffer, SAMPLE_NUM * 2 * tmp->header.nChannels, &i);

            if(tmp->loop && (i < SAMPLE_NUM * 2 * tmp->header.nChannels)){
                f_lseek(&fil, sizeof(struct wavHeader_t));
                f_read(&fil, buffer + (i >> 2), SAMPLE_NUM * 2 * tmp->header.nChannels - i, &i);
            }

            if(tmp->header.nChannels > 1){
                for(j = 0; j < SAMPLE_NUM; j++){
                    buffer[j] = buffer[j * 2];
                }
            }

            arm_q15_to_q31((q15_t*)buffer, tmp->dataBuffer[bufferIndex], SAMPLE_NUM);

            bufferIndex = !bufferIndex;
        }

    }while(i);

    arm_fill_q31(0, tmp->dataBuffer[0], SAMPLE_NUM);
    arm_fill_q31(0, tmp->dataBuffer[1], SAMPLE_NUM);

    vTaskDelete(NULL);
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
	            2048, opaque, tskIDLE_PRIORITY + 1, NULL);

    return (struct Effect_t*)opaque;
}

