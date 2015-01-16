#ifndef __WAVPLAYER_H__
#define __WAVPLAYER_H__

#include "FreeRTOS.h"
#include "semphr.h"

#include "base-effect.h"

struct WavPlayer_t{
    struct Effect_t parent;
    struct parameter_t volume;
    q31_t cache; 

    uint32_t loop;

    char *filename;
    struct wavHeader_t header;

    uint32_t bufferIndex;
    q31_t dataBuffer[2][SAMPLE_NUM];

    xSemaphoreHandle Read_Hold;
};

struct Effect_t* new_WavPlayer(struct WavPlayer_t* opaque);

#endif //__WAVPLAYER_H__
