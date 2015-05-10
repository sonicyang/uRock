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

uint16_t outputBuffer[BUFFER_NUM][SAMPLE_NUM * 2]; //L-R channels -> 2x buffer length
uint32_t inputBuffer[BUFFER_NUM][SAMPLE_NUM];

uint8_t receivePipeHead = 0;
uint8_t transmitPipeHead = 0;
uint8_t pipeUsage = 0;
q31_t signalPipe[PIPE_LENGTH][SAMPLE_NUM] __attribute__ ((section (".ccmram"))) = {{255, 255, 255}};

struct Effect_t *effectList[STAGE_NUM];
uint8_t ValueForEachStage[STAGE_NUM][MAX_EFFECT_PARAM];
    
int16_t wavData[4200];

extern uint8_t potRawValues[4];

void SignalProcessingUnit(void const * argument){
    uint32_t i;

    for(i = 0; i < STAGE_NUM; i++){
        effectList[i] = NULL;
    }

//    effectList[0] = new_Volume(); 

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
                    effectList[i]->func(signalPipe[(receivePipeHead - 1 - i) & (PIPE_LENGTH - 1)], effectList[i]); //Hack, modulating 16 -> using mask only avliable to 2^n
                }
                //TODO: Use empty function call;
            }
        }
    }

    while(1);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai){
    uint16_t i;

    for(i = 0; i < 256; i++)
        signalPipe[receivePipeHead & (PIPE_LENGTH - 1)][i] = (inputBuffer[0][i] << 8) - (NORM_VALUE);

    receivePipeHead++;

    pipeUsage++;

    osSemaphoreRelease(SPUH_id);
    return;
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai){
    uint16_t i;
    for(i = 0; i < 256; i++)
        signalPipe[receivePipeHead & (PIPE_LENGTH - 1)][i] = (inputBuffer[1][i] << 8) - (NORM_VALUE);

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
        outputBuffer[0][(i << 1)] = (signalPipe[transmitPipeHead & (PIPE_LENGTH - 1)][i] >> 16);
        outputBuffer[0][(i << 1) + 1] = (signalPipe[transmitPipeHead & (PIPE_LENGTH - 1)][i] >> 16);
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
        outputBuffer[1][(i << 1)] = (signalPipe[transmitPipeHead & (PIPE_LENGTH - 1)][i] >> 16);
        outputBuffer[1][(i << 1) + 1] = (signalPipe[transmitPipeHead & (PIPE_LENGTH - 1)][i] >> 16);
    }

    transmitPipeHead++;

    pipeUsage--;
    return;
}

void attachEffect(uint32_t stage, EffectType_t effectType){
	if(effectList[stage])
		effectList[stage]->del(effectList[stage]);
    //XXX: Probably need to return to RTOS for truly releasing memory
    

	switch(effectType){
        case NONE:
            effectList[stage] = NULL;
            break;
        case VOLUME:
            effectList[stage] = new_Volume();
            break;
        case COMPRESSOR:
            effectList[stage] = new_Compressor();
            break;
        case DISTORTION:
            effectList[stage] = new_Distortion();
            break;
        case OVERDRIVE:
            effectList[stage] = new_Overdrive();
            break;
        case DELAY:
            effectList[stage] = new_Delay();
            break;
        case REVERB:
            effectList[stage] = new_Reverb();
            break;
        case FLANGER:
            effectList[stage] = new_Flanger();
            break;
        case EQULIZER:
            effectList[stage] = new_Equalizer();
            break;
        default:
            assert_param(0);
            effectList[stage] = NULL;
            break;
	}
     
    return;
}

const char *cvtToEffectName(EffectType_t ee){
	switch(ee){
        case NONE:
            return "None";
        case VOLUME:
            return "Volume";
        case COMPRESSOR:
            return "Compressor";
        case DISTORTION:
            return "Distortion";
        case OVERDRIVE:
            return "OverDrive";
        case DELAY:
            return "Delay";
        case REVERB:
            return "Reverb";
        case FLANGER:
            return "Flanger";
        case EQULIZER:
            return "Equalizer";
        default:
            return "Error Cvt";
	}
    return "Error Cvt";
}

const char *retriveEffectStageName(uint32_t stage){
    if(effectList[stage])
        return effectList[stage]->name;
    return "";
}
