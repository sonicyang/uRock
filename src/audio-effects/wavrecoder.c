#include "wavrecoder.h"
#include "helper.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "ff.h"

const struct wavHeader_t wHeader = {
                                    .filID = "RIFF",
                                    .filSize = 4 + 24 + 8,
                                    .waveID = "WAVE",
                                    .fmtID = "fmt ",
                                    .fmtSize = 16,
                                    .wFormatTag = 0x0001,
                                    .nChannels = 1,
                                    .nSamplesPerSec = 44100,
                                    .nAvgBytesPerSec = 88200,
                                    .nBlockAlign = 2,
                                    .wBitsPerSample = 16,
                                    .dataID = "data",
                                    .dataSize = 88200
                                    };

void writeWaveTask(void *pvParameters){
    struct WavRecoder_t *tmp = (struct WavRecoder_t*)pvParameters;

    uint16_t buffer[SAMPLE_NUM];
    uint32_t bufferIndex = 0;
    uint16_t wavDataWritenExt = 4 + 24 + 8;
    uint16_t wavDataWriten = 0;
    uint32_t cnt = 0;
    UINT i;

    FIL fil;

    if (f_open(&fil, tmp->filename, FA_OPEN_ALWAYS | FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) for(;;);

    f_write(&fil, &wHeader, sizeof(struct wavHeader_t), &i);
    if(i < sizeof(struct wavHeader_t)) //Check Writen
        vTaskDelete(NULL);

    do{
        if(xSemaphoreTake(tmp->Write_Hold, portMAX_DELAY)){
            arm_q31_to_q15(tmp->dataBuffer[bufferIndex], (q15_t*)buffer, SAMPLE_NUM);

            f_write(&fil, buffer, SAMPLE_NUM * 2, &i);

            wavDataWriten += SAMPLE_NUM * 2;
            wavDataWritenExt += SAMPLE_NUM * 2;


            bufferIndex = !bufferIndex;
            cnt++;
        }

        if(cnt > 200){
            f_lseek(&fil, 4);
            f_write(&fil, &wavDataWritenExt, 4, &i);

            f_lseek(&fil, 40);
            f_write(&fil, &wavDataWriten, 4, &i);

            f_lseek(&fil, f_size(&fil));

            f_sync(&fil);
            cnt = 0;
        }

    }while(1);

    arm_fill_q31(0, tmp->dataBuffer[0], SAMPLE_NUM);
    arm_fill_q31(0, tmp->dataBuffer[1], SAMPLE_NUM);

    vTaskDelete(NULL);
}

void WavRecoder(q31_t* pData, void *opaque){
    struct WavRecoder_t *tmp = (struct WavRecoder_t*)opaque;

    arm_copy_q31(pData, tmp->dataBuffer[tmp->bufferIndex], SAMPLE_NUM);
    tmp->bufferIndex = !tmp->bufferIndex;

    xSemaphoreGive(tmp->Write_Hold);

    return;
}

void delete_WavRecoder(void *opaque){
    struct WavRecoder_t *tmp = (struct WavRecoder_t*)opaque;

    vTaskDelete(tmp->wwt);
    vSemaphoreDelete(tmp->Write_Hold);

    vPortFree(tmp);
    return;
}

void adjust_WavRecoder(void *opaque, uint8_t* values){
    return;
}

void getParam_WavRecoder(void *opaque, struct parameter_t *param[], uint8_t* paramNum){
    *paramNum = 0;
    return;
}

struct Effect_t* new_WavRecoder(){
    struct WavRecoder_t* tmp = pvPortMalloc(sizeof(struct WavRecoder_t));

    strcpy(tmp->parent.name, "WavRecoder");
    tmp->parent.func = WavRecoder;
    tmp->parent.del = delete_WavRecoder;
    tmp->parent.adj = adjust_WavRecoder;
    tmp->parent.getParam = getParam_WavRecoder;

    /* FIXME Hardcode */
    tmp->filename = "0:/rec.wav";

    tmp->bufferIndex = 0;
    
    tmp->Write_Hold = xSemaphoreCreateBinary();
    xSemaphoreGive(tmp->Write_Hold);

	xTaskCreate(writeWaveTask,
	            (signed char*)"WWT",
	            2048, tmp, tskIDLE_PRIORITY + 2, &tmp->wwt);

    return (struct Effect_t*)tmp;
}

