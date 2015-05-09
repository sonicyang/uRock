#include "cmsis_os.h"

#include "setting.h"
#include "base-effect.h"

#include "spu.h"

#include "volume.h"
#include "compressor.h"

#include "distortion.h"
#include "overdrive.h"

#include "equalizer.h"

#include "delay.h"
#include "reverb.h"

#include "phaser.h"
#include "flanger.h"

//#include "wavplayer.h"
//#include "wavrecoder.h"

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockB1;

osSemaphoreId SPUH_id; 

uint16_t outputBuffer[BUFFER_NUM][SAMPLE_NUM * 2];
uint32_t inputBuffer[BUFFER_NUM][SAMPLE_NUM];

uint8_t receivePipeHead = 0;
uint8_t transmitPipeHead = 0;
uint8_t pipeUsage = 0;
q31_t signalPipe[STAGE_NUM * 2][SAMPLE_NUM] __attribute__ ((section (".ccmram"))) = {{255, 255, 255}};


struct Effect_t *effectList[STAGE_NUM];
uint8_t ValueForEachStage[STAGE_NUM][MAX_EFFECT_PARAM];
int8_t controllingStage = 0;
    
int16_t wavData[4200];

extern uint8_t potRawValues[4];

void SignalProcessingUnit(void const * argument){
    uint32_t i;

    for(i = 0; i < STAGE_NUM; i++){
        effectList[i] = NULL;
    }

    effectList[0] = new_Volume(); 

    /* Semaphore Blocker setup*/ 
    osSemaphoreDef(SPUH);
    SPUH_id = osSemaphoreCreate(osSemaphore(SPUH), 1);

    /* Start Transmission and reception */
    HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t*)inputBuffer[0], 512); //Salve goes before Master
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*)outputBuffer[0], 1024);

    /* Process */
    while(1){
        if(osSemaphoreWait(SPUH_id, osWaitForever) == osOK){
            for(i = 0; i < STAGE_NUM; i++){
                if(effectList[i] != NULL){
                    effectList[i]->func(signalPipe[(receivePipeHead - 1 - i) & 0xF], effectList[i]); //Hack, modulating 16 -> using mask
                }
                //TODO: Use empty function call;
            }
        }
		effectList[controllingStage]->adj(effectList[controllingStage], potRawValues);
    }

    while(1);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai){
    uint16_t i;

    for(i = 0; i < 256; i++)
        signalPipe[receivePipeHead & 0xF][i] = (inputBuffer[0][i] << 8) - (NORM_VALUE);

    receivePipeHead++;

    pipeUsage++;

    osSemaphoreRelease(SPUH_id);
    return;
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai){
    uint16_t i;
    for(i = 0; i < 256; i++)
        signalPipe[receivePipeHead & 0xF][i] = (inputBuffer[1][i] << 8) - (NORM_VALUE);

    receivePipeHead++;

    pipeUsage++;

    osSemaphoreRelease(SPUH_id);
    return;
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){
    uint16_t i;

    if(pipeUsage <= 8)
        return;

    for(i = 0; i < 256; i++){
        outputBuffer[0][(i << 1)] = (signalPipe[transmitPipeHead & 0xF][i] >> 16);
        outputBuffer[0][(i << 1) + 1] = (signalPipe[transmitPipeHead & 0xF][i] >> 16);
    }

    transmitPipeHead++;
    if(transmitPipeHead == 16)
        transmitPipeHead = 0;

    pipeUsage--;
    return;
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai){
    uint16_t i;

    if(pipeUsage <= 8)
        return;

    for(i = 0; i < 256; i++){
        outputBuffer[1][(i << 1)] = (signalPipe[transmitPipeHead & 0xF][i] >> 16);
        outputBuffer[1][(i << 1) + 1] = (signalPipe[transmitPipeHead & 0xF][i] >> 16);
    }

    transmitPipeHead++;

    pipeUsage--;
    return;
}
