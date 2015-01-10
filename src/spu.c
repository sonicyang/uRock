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

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac2;

TIM_HandleTypeDef htim2;

xSemaphoreHandle SPU_Hold;
volatile uint16_t SignalBuffer[BUFFER_NUM][SAMPLE_NUM]; 
q31_t SignalPipe[STAGE_NUM][SAMPLE_NUM];

#define EFFECT_NUM 3
int8_t EffectStages[STAGE_NUM];
struct Effect_t *EffectList[EFFECT_NUM];
uint8_t ValueForEachStage[STAGE_NUM][3];
int8_t controllingStage;

struct Volume_t vol;
struct Distortion_t distor;
struct Overdrive_t overdrive;
struct Reverb_t delay;
struct Flanger_t flanger;
struct Phaser_t phaser;
struct Equalizer_t equalizer;
struct Compressor_t compressor;

void SignalProcessingUnit(void *pvParameters){
    uint32_t index = 0;
    uint32_t pipeindex = 0;
    uint32_t i;

    for(i = 0; i < EFFECT_NUM; i++){
        EffectList[i] = NULL;
    }

    /* Effect Stage Setting*/ 

    //EffectList[0] = new_Volume(&vol);
    //EffectList[0] = new_Distortion(&distor);
    EffectList[0] = new_Compressor(&compressor);
    EffectList[1] = new_Overdrive(&overdrive);
    EffectList[2] = NULL;
    //EffectList[0] = new_Phaser(&phaser);
    //EffectList[2] = new_Equalizer(&equalizer);
    //EffectList[3] = new_Reverb(&delay);
    //EffectList[0] = new_Flanger(&flanger);

    for(i = 0; i < STAGE_NUM; i++){
        EffectStages[i] = 2;
    }

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
                if(EffectList[EffectStages[i]] != NULL){
                    EffectList[EffectStages[i]]->func(SignalPipe[(pipeindex - i) % STAGE_NUM], EffectList[EffectStages[i]]);
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
