#ifndef __WAVRECODER_H__
#define __WAVRECODER_H__

#include "FreeRTOS.h"
#include "semphr.h"

#include "base-effect.h"

struct WavRecoder_t{
    struct Effect_t parent;

    char *filename;

    uint32_t bufferIndex;
    q31_t dataBuffer[2][SAMPLE_NUM];

    xSemaphoreHandle Write_Hold;
};

struct Effect_t* new_WavRecoder(struct WavRecoder_t* opaque);

#endif //__WAVRECODER_H__
