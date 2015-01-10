#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "setting.h"
#include "base-effect.h"

#include "volume.h"
#include "compressor.h"

#include "distortion.h"
#include "overdrive.h"

#include "equalizer.h"

#include "delay.h"
#include "reverb.h"

#include "phaser.h"
#include "flanger.h"

#include "ff.h"

extern char SD_Path[4];
extern FATFS FatFs;
extern FIL fil;

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac2;

TIM_HandleTypeDef htim2;

xSemaphoreHandle SPU_Hold;
volatile uint16_t SignalBuffer[BUFFER_NUM][SAMPLE_NUM]; 
q31_t SignalPipe[STAGE_NUM][SAMPLE_NUM];
struct Effect_t *EffectStages[STAGE_NUM];

struct Volume_t vol;
struct Distortion_t distor;
struct Overdrive_t overdrive;
struct Reverb_t delay;
struct Flanger_t flanger;
struct Phaser_t phaser;
struct Equalizer_t equalizer;
struct Compressor_t compressor;

struct wavHeader_t{
    char        filID[4];
    uint32_t    filSize;
    char        waveID[4];
    char        fmtID[4];
    uint32_t    fmtSize;
    uint16_t    wFormatTag;
    uint16_t    nChannels;
    uint32_t    nSamplesPerSec;
    uint32_t    nAvgBytesPerSec;
    uint16_t    nBlockAlign;
    uint16_t    wBitsPerSample;
    char        dataID[4];
    uint32_t    dataSize;
} __attribute__((packed)) wavHeader;

void SignalProcessingUnit(void *pvParameters){
    uint32_t index = 0;
    uint32_t pipeindex = 0;
    uint32_t i;

   if (f_mount(&FatFs, SD_Path, 1) == FR_OK) {
         //Try to open file
         if (f_open(&fil, "0:/tst.wav", FA_OPEN_ALWAYS | FA_READ) == FR_OK) {
             f_read(&fil, &wavHeader, sizeof(wavHeader), &i);

             f_close(&fil);
         }

         //Unmount drive, don't forget this!
         f_mount(0, SD_Path, 1);
     } 

    for(i = 0; i < STAGE_NUM; i++){
        EffectStages[i] = NULL;
    }

    /* Effect Stage Setting*/ 

    //EffectStages[0] = new_Volume(&vol);
    //EffectStages[0] = new_Distortion(&distor);
    //EffectStages[0] = new_Compressor(&compressor);
    //EffectStages[1] = new_Overdrive(&overdrive);
    //EffectStages[0] = new_Phaser(&phaser);
    //EffectStages[2] = new_Equalizer(&equalizer);
    //EffectStages[3] = new_Reverb(&delay);
    //EffectStages[0] = new_Flanger(&flanger);
    //
    SPU_Hold = xSemaphoreCreateBinary();

    /* Init */
    HAL_TIM_Base_Start(&htim2);
    HAL_ADC_Start_DMA_DoubleBuffer(&hadc1, (uint32_t*)SignalBuffer[0], (uint32_t*)SignalBuffer[1], SAMPLE_NUM);
    HAL_DAC_Start_DMA_DoubleBuffer(&hdac, DAC_CHANNEL_2, (uint32_t*) SignalBuffer[1], (uint32_t*) SignalBuffer[2], SAMPLE_NUM, DAC_ALIGN_12B_R);

    /* Process */
    while(1){
        if(xSemaphoreTake(SPU_Hold, portMAX_DELAY)){

            NormalizeData(SignalBuffer[index], SignalPipe[pipeindex]);
            
            for(i = 0; i < STAGE_NUM; i++){
                if(EffectStages[i] != NULL){
                    EffectStages[i]->func(SignalPipe[(pipeindex - i) % STAGE_NUM], EffectStages[i]);
                }
            }

            DenormalizeData(SignalPipe[(pipeindex - STAGE_NUM + 1) % STAGE_NUM], SignalBuffer[(index - 1) % BUFFER_NUM]);

            index+=1;
            if(index >= BUFFER_NUM)
                index = 0;
            pipeindex+=1;
            if(pipeindex >= STAGE_NUM)
                pipeindex = 0;
        }
    }

    while(1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    static uint32_t index = 0;
    static signed portBASE_TYPE xHigherPriorityTaskWoken;

    if(hadc->Instance == ADC1){
        index += 2;
        if(index >= BUFFER_NUM)
            index = 0;

        xHigherPriorityTaskWoken = pdFALSE;

        HAL_DMAEx_ChangeMemory(&hdma_adc1, (uint32_t)SignalBuffer[index], MEMORY0);

        xSemaphoreGiveFromISR(SPU_Hold, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken != pdFALSE)
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    return;  
}

void HAL_ADC_ConvM1CpltCallback(ADC_HandleTypeDef* hadc){
    static uint32_t index = 1;
    static signed portBASE_TYPE xHigherPriorityTaskWoken;

    if(hadc->Instance == ADC1){
        index += 2;
        if(index >= BUFFER_NUM)
            index = 1;
        
        xHigherPriorityTaskWoken = pdFALSE;

        HAL_DMAEx_ChangeMemory(&hdma_adc1, (uint32_t)SignalBuffer[index], MEMORY1);

        xSemaphoreGiveFromISR(SPU_Hold, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken != pdFALSE)
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return;  
}

void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac){
    static uint32_t index = 1;

    index += 2;
    if(index >= BUFFER_NUM)
        index = 1;

    HAL_DMAEx_ChangeMemory(&hdma_dac2, (uint32_t)SignalBuffer[index], MEMORY0);

    return;  
}

void HAL_DACEx_ConvM1CpltCallbackCh2(DAC_HandleTypeDef* hdac){
    static uint32_t index = 2;

    index += 2;
    if(index >= BUFFER_NUM)
        index = 0;

    HAL_DMAEx_ChangeMemory(&hdma_dac2, (uint32_t)SignalBuffer[index], MEMORY1);

    return;  
}
