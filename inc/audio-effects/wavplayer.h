#ifndef __WAVPLAYER_H__
#define __WAVPLAYER_H__

#include "FreeRTOS.h"
#include "semphr.h"

#include "base-effect.h"

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
} __attribute__((packed));

struct WavPlayer_t{
    struct Effect_t parent;
    struct parameter_t volume;
    q31_t cache; 

    char *filename;
    struct wavHeader_t header;

    uint32_t bufferIndex;
    q31_t dataBuffer[2][SAMPLE_NUM];

    xSemaphoreHandle Read_Hold;
};

struct Effect_t* new_WavPlayer(struct WavPlayer_t* opaque);

#endif //__WAVPLAYER_H__
