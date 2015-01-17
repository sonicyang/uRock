#ifndef __WAVPLAYER_H__
#define __WAVPLAYER_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "base-effect.h"

#include "ff.h"

struct WavPlayer_t{
    struct Effect_t parent;
    struct parameter_t volume;
    q31_t cache; 

    uint32_t loop;

    FIL fil;
    char *filename;
    struct wavHeader_t header;

    uint32_t bufferIndex;
    q31_t dataBuffer[2][SAMPLE_NUM];
    
    xTaskHandle rwt;
    xSemaphoreHandle Read_Hold;
};

struct Effect_t* new_WavPlayer();

#endif //__WAVPLAYER_H__
