#ifndef __AUDIO_EFFECT_H__
#define __AUDIO_EFFECT_H__

#include <stdint.h>

#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery_sdram.h"

#include "math.h"

#include "arm_math.h"

#include "setting.h"

#include "gfx.h"

uint32_t allocateDelayLine();
void releaseDelayLine(uint32_t);

struct parameter_t{
    char* name;
    float value;
    float upperBound;
    float lowerBound;
};

typedef void(*EffectFunc)(q31_t*, void*);
typedef void(*DeleteFunc)(void*);
typedef void(*adjustFunc)(void*, uint8_t*);
typedef void(*getParamFunc)(void*,struct parameter_t**, uint8_t*);

struct Effect_t{
    char name[16];
    struct EffectType_t* FXid;
    EffectFunc func;
    DeleteFunc del;
    adjustFunc adj;
    getParamFunc getParam;
};

typedef struct Effect_t*(*newEffectFunc)(void);

struct EffectType_t{
    const char name[16];
    const char* image;
    gdispImage cacheImage;
    newEffectFunc   Init;
};

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

#endif //__AUDIO_EFFECT_H__
