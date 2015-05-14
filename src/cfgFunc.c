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

void SaveStageSetting(uint32_t saveSlot){
    FIL fil;
    uint32_t i;

    switch (saveSlot) {
        case 0:
            do{
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }while(f_open(&fil, "0:/config0", FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_WRITE) != FR_OK);
            break;
        case 1:
            do{
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }while(f_open(&fil, "0:/config1", FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_WRITE) != FR_OK);
            break;
        case 2:
            do{
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }while(f_open(&fil, "0:/config2", FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_WRITE) != FR_OK);
            break;
        case 3:
            do{
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }while(f_open(&fil, "0:/config3", FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_WRITE) != FR_OK);
            break;
        default:
            /* XXX Bad hack */
            while (1)
                ;
            break;
    }

    for (i = 0; i < STAGE_NUM; ++i) {
        uint8_t paramNum;
        struct parameter_t* params[MAX_EFFECT_PARAM];
        char buf[32];
        uint32_t j;

        if (retriveStagedEffect(i)){
            retriveStagedEffect(i)->getParam((void*)retriveStagedEffect(i), params, &paramNum);

            f_write(&fil,
                retriveStagedEffect(i)->name,
                strlen(retriveStagedEffect(i)->name),
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
    f_sync(&fil);
    f_close(&fil);
}

void ReadStageSetting(uint32_t saveSlot){
    FIL fil;
    char c;
    uint8_t paramValues[MAX_EFFECT_PARAM];
    char buf[32];
    uint32_t bufPos;
    uint32_t i;
    char* stageNamePtr;
    char* paramPtrs[MAX_EFFECT_PARAM];
    struct parameter_t* params[MAX_EFFECT_PARAM];
    uint8_t paramNum;

    switch (saveSlot) {
        case 0:
            do{
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }while(f_open(&fil, "0:/config0", FA_OPEN_ALWAYS | FA_READ) != FR_OK);
            break;
        case 1:
            do{
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }while(f_open(&fil, "0:/config1", FA_OPEN_ALWAYS | FA_READ) != FR_OK);
            break;
        case 2:
            do{
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }while(f_open(&fil, "0:/config2", FA_OPEN_ALWAYS | FA_READ) != FR_OK);
            break;
        case 3:
            do{
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }while(f_open(&fil, "0:/config3", FA_OPEN_ALWAYS | FA_READ) != FR_OK);
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

        attachEffect(i, cvtToEffectId(stageNamePtr));

        //XXX: to Brainless mode, effect will ignore extra param given
        if(!retriveStagedEffect(i))
            continue;
        retriveStagedEffect(i)->getParam((void*)retriveStagedEffect(i), params, &paramNum);

        for (int i = 0; i < paramNum; ++i)
            paramValues[i] = map(atof(paramPtrs[i]), params[0]->lowerBound, params[0]->upperBound, 0, 255);

        retriveStagedEffect(i)->adj((void*)retriveStagedEffect(i), paramValues);
    }

    f_close(&fil);
}
