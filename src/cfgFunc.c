#include "cfgFunc.h"
#include "spu.h"
#include "setting.h"
#include "base-effect.h"
#include "helper.h"

#include "tab_select_effect.h"

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

#include <stdint.h>
#include <stdio.h>

extern char SD_Path[4];
extern FATFS FatFs;

extern struct Effect_t *EffectList[EFFECT_NUM];
extern uint8_t ValueForEachStage[STAGE_NUM][MAX_EFFECT_PARAM];
extern int8_t controllingStage;

static uint32_t cvtToEffectId(const char* name){
    if (strcmp(name, "Volume") == 0)
        return VOL;

    if (strcmp(name, "Compressor") == 0)
        return COMP;

    if (strcmp(name, "Distortion") == 0)
        return DISTOR;

    if (strcmp(name, "Overdrive") == 0)
        return OVERDR;

    if (strcmp(name, "Delay") == 0)
        return DELAY;

    if (strcmp(name, "Reverb") == 0)
        return REVERB;

    if (strcmp(name, "Flanger") == 0)
        return FLANGE;

    if (strcmp(name, "Equalizer") == 0)
        return EQULIZ;

    if (strcmp(name, "None") == 0)
        return NONE;

    return NONE;
}

void SaveStageSetting(uint32_t saveSlot){
    FIL fil;
    uint32_t i;

    switch (saveSlot) {
    case 0:
        if (f_open(&fil, "0:/config0", FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) for(;;);
        break;
    case 1:
        if (f_open(&fil, "0:/config1", FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) for(;;);
        break;
    case 2:
        if (f_open(&fil, "0:/config2", FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) for(;;);
        break;
    case 3:
        if (f_open(&fil, "0:/config3", FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) for(;;);
        break;
    default:
        /* XXX Bad hack */
        while (1)
            ;
        break;
    }

    for (i = 0; i < STAGE_NUM; ++i) {
        uint8_t paramNum;
        struct parameter_t* params[3];
        char buf[5];
        uint32_t j;

        if (EffectList[i]) {
            EffectList[i]->getParam(
                EffectList[i], params, &paramNum);

            f_write(&fil,
                EffectList[i]->name,
                strlen(EffectList[i]->name),
                NULL);

            for (j = 0; j < paramNum; ++j) {
                if (params[j]->value < 0)
                    f_write(&fil, " ", 1, NULL);

                ftoa(params[j]->value, buf, 2);

                f_write(&fil,
                    buf,
                    strlen(buf),
                    NULL);
            }

            /* Fill with zero */
            for (; j < 3; ++j) {
                f_write(&fil, " ", 1, NULL);
                f_write(&fil,
                    "0",
                    1,
                    NULL);
            }

            f_write(&fil, "\n", 1, NULL);
        } else {
            f_write(&fil, "None\n", 5, NULL);
        }
    }

    f_close(&fil);
}

void ReadStageSetting(uint32_t saveSlot){
    FIL fil;
    char c;
    uint8_t paramValues[3];
    char buf[30];
    uint32_t bufPos;
    uint32_t i;
    char* stageNamePtr;
    char* paramPtrs[3];
    struct Effect_t *prevEffect;
    struct parameter_t* params[3];
    uint8_t paramNum;

    switch (saveSlot) {
    case 0:
        if (f_open(&fil, "0:/config0", FA_OPEN_ALWAYS | FA_READ) != FR_OK) for(;;);
        break;
    case 1:
        if (f_open(&fil, "0:/config1", FA_OPEN_ALWAYS | FA_READ) != FR_OK) for(;;);
        break;
    case 2:
        if (f_open(&fil, "0:/config2", FA_OPEN_ALWAYS | FA_READ) != FR_OK) for(;;);
        break;
    case 3:
        if (f_open(&fil, "0:/config3", FA_OPEN_ALWAYS | FA_READ) != FR_OK) for(;;);
        break;
    default:
        /* XXX Bad hack */
        while (1)
            ;
        break;
    }

    for (i = 0; i < STAGE_NUM; ++i) {
        bufPos = 0;
        stageNamePtr = buf;

        for (int i = 0; i < 3; ++i)
            paramPtrs[i] = NULL;

        /* Read line */
        while (1) {
            f_read(&fil, &c, 1, NULL);
            if ((c == '\n') || (c == EOF) || (c == '\0')) {
                buf[bufPos] = '\0';
                break;
            } else if (c == ' ') {
                buf[bufPos] = '\0';

                if (!paramPtrs[0])
                    paramPtrs[0] = &(buf[bufPos + 1]);
                else if (!paramPtrs[1])
                    paramPtrs[1] = &(buf[bufPos + 1]);
                else if (!paramPtrs[2])
                    paramPtrs[2] = &(buf[bufPos + 1]);
            } else {
                buf[bufPos] = c;
            }

            ++bufPos;
        }

        prevEffect = EffectList[i];
        switch (cvtToEffectId(stageNamePtr)) {
        case VOL:
            EffectList[i] = new_Volume();
            break;
        case COMP:
            EffectList[i] = new_Compressor();
            break;
        case DISTOR:
            EffectList[i] = new_Distortion();
            break;
        case OVERDR:
            EffectList[i] = new_Overdrive();
            break;
        case DELAY:
            EffectList[i] = new_Delay();
            break;
        case REVERB:
            EffectList[i] = new_Reverb();
            break;
        case FLANGE:
            EffectList[i] = new_Flanger();
            break;
        case EQULIZ:
            EffectList[i] = new_Equalizer();
            break;
        default:
            EffectList[i] = NULL;
            break;
        }

        if (prevEffect)
            prevEffect->del(prevEffect);

        if (!EffectList[i])
            continue;

        EffectList[i]->getParam(EffectList[i], params, &paramNum);

        for (int i = 0; i < paramNum; ++i)
            paramValues[i] = map(atof(paramPtrs[i]), params[0]->lowerBound, params[0]->upperBound, 0, 255);

        EffectList[i]->adj(EffectList[i], paramValues);
    }

    f_close(&fil);
}
