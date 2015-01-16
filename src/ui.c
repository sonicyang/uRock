#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "spu.h"
#include "setting.h"
#include "base-effect.h"
#include "helper.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "gfxconf.h"
#include "gfx.h"
#include "src/gwin/sys_defs.h"

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

#define TAB_GROUP_1 0

extern char SD_Path[4];
extern FATFS FatFs;
extern FIL fil;

DMA_HandleTypeDef hdma_adc2;
ADC_HandleTypeDef hadc2;

extern struct Effect_t *EffectList[EFFECT_NUM];
extern uint8_t ValueForEachStage[STAGE_NUM][MAX_EFFECT_PARAM];
extern int8_t controllingStage;

enum {
    NONE,
    VOL,
    COMP,
    DISTOR,
    OVERDR,
    DELAY,
    REVERB,
    FLANGE,
    EQULIZ,
    BACK,
    EFFECT_TYPE_NUM
};

enum{
    LIST_TAB,
    PARAM_TAB,
    SELECT_EFFECT_TAB
};

static GHandle label_uRock;
static GHandle btn_effectIndicate[STAGE_NUM];
static GHandle label_effectName[STAGE_NUM];
static GHandle btn_effectSwitch[STAGE_NUM];
static GHandle label_effectTitle;
static GHandle btn_prevStage;
static GHandle btn_nextStage;
static GHandle label_param[MAX_EFFECT_PARAM];
static GHandle vbar_param[MAX_EFFECT_PARAM];
//static GHandle btn_StageWidget;
//static GHandle btn_SelectEffectWidget;
static GHandle btn_effectTypes[EFFECT_TYPE_NUM];

static char defaultName[8] = "";

static GListener gl;

static uint32_t tabState = 0;

uint8_t potValues[2][4];

static char *cvtToEffectName(uint32_t ee){
    switch(ee){
    case VOL:
        return "Volume";
    case COMP:
        return "Compressor";
    case DISTOR:
        return "Distortion";
    case OVERDR:
        return "OverDrive";
    case DELAY:
        return "Delay";
    case REVERB:
        return "Reverb";
    case FLANGE:
        return "Flanger";
    case EQULIZ:
        return "Equalizer";
    default:
        return "None";
    }
    return "None";
}

static void createWidgets(void) {
    uint32_t i;
    GWidgetInit wi;

    /* StageTab */
    gwinWidgetClearInit(&wi);
    wi.g.show = TRUE;
    wi.g.x = 0;
    wi.g.y = 0;
    wi.g.width = 100;
    wi.g.height = 20;
    wi.text = "uRock";
    label_uRock = gwinLabelCreate(NULL, &wi);

    for(i = 0; i < STAGE_NUM; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = TRUE;
        wi.g.x = 10;
        wi.g.y = 50 + 40 * i;
        wi.g.width = 30;
        wi.g.height = 30;
        wi.text = "";
        btn_effectIndicate[i] = gwinButtonCreate(NULL, &wi);

        gwinWidgetClearInit(&wi);
        wi.g.show = TRUE;
        wi.g.x = 50;
        wi.g.y = 50 + 40 * i;
        wi.g.width = 140;
        wi.g.height = 30;
        wi.text = "";
        label_effectName[i] = gwinLabelCreate(NULL, &wi);

        gwinWidgetClearInit(&wi);
        wi.g.show = TRUE;
        wi.g.x = 200;
        wi.g.y = 50 + 40 * i;
        wi.g.width = 30;
        wi.g.height = 30;
        wi.text = "";
        btn_effectSwitch[i] = gwinButtonCreate(NULL, &wi);
    }

    /* ParamTab */
    gwinWidgetClearInit(&wi);
    wi.g.show = TRUE;
    wi.g.x = 15;
    wi.g.y = 30;
    wi.g.width = (240 - 20);
    wi.g.height = 25;
    wi.text = "";
    label_effectTitle = gwinLabelCreate(NULL, &wi);

    for(i = 0; i < MAX_EFFECT_PARAM; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = TRUE;
        wi.g.x = 5;
        wi.g.y = 60 + i * 60;
        wi.g.width = (240 - 10);
        wi.g.height = 25;
        wi.text = defaultName;
        label_param[i] = gwinLabelCreate(NULL, &wi);

        wi.g.y += 25;
        vbar_param[i] = gwinProgressbarCreate(NULL, &wi);
    }

    /* SelectStageTab */
    for(i = 0; i < EFFECT_TYPE_NUM - 1; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = TRUE;
        wi.g.x = 5;
        wi.g.y = 5 + 25 * i;
        wi.g.width = 230;
        wi.g.height = 20;
        wi.text = cvtToEffectName(i);
        btn_effectTypes[i] = gwinButtonCreate(NULL, &wi);
    }
    gwinWidgetClearInit(&wi);
    wi.g.show = TRUE;
    wi.g.x = 5;
    wi.g.y = 290;
    wi.g.width = 230;
    wi.g.height = 20;
    wi.text = "Back";
    btn_effectTypes[EFFECT_TYPE_NUM - 1] = gwinButtonCreate(NULL, &wi);
}

void SwitchTab(uint32_t tab){
    uint32_t i;

    gwinSetVisible(label_uRock, FALSE);
    for(i = 0; i < STAGE_NUM; i++){
        gwinSetVisible(btn_effectIndicate[i], FALSE);
        gwinSetVisible(label_effectName[i], FALSE);
        gwinSetVisible(btn_effectSwitch[i], FALSE);
    }

    gwinSetVisible(label_effectTitle, FALSE);
    gwinSetVisible(btn_prevStage, FALSE);
    gwinSetVisible(btn_nextStage, FALSE);
    for(i = 0; i < MAX_EFFECT_PARAM; i++){
        gwinSetVisible(label_param[i], FALSE);
        gwinSetVisible(vbar_param[i], FALSE);
    }

    for(i = 0; i < EFFECT_TYPE_NUM; i++){
        gwinSetVisible(btn_effectTypes[i], FALSE);
    }

    if(tab == LIST_TAB){
        gwinSetVisible(label_uRock, TRUE);
        for(i = 0; i < STAGE_NUM; i++){
            gwinSetVisible(btn_effectIndicate[i], TRUE);
            gwinSetVisible(label_effectName[i], TRUE);
            gwinSetVisible(btn_effectSwitch[i], TRUE);
        }

        tabState = LIST_TAB;
    }else if (tab == PARAM_TAB) {
        gwinSetVisible(label_effectTitle, TRUE);
        for(i = 0; i < MAX_EFFECT_PARAM; i++){
            gwinSetVisible(label_param[i], TRUE);
            gwinSetVisible(vbar_param[i], TRUE);
        }

        tabState = PARAM_TAB;
    } else if (tab == SELECT_EFFECT_TAB) {
        for(i = 0; i < EFFECT_TYPE_NUM; i++){
            gwinSetVisible(btn_effectTypes[i], TRUE);
        }

        tabState = SELECT_EFFECT_TAB;
    }
}

static void RefreshScreen(void){
    char buf[32];
    struct parameter_t *parameterList[5];
    uint8_t paraNum;
    uint32_t i = 0;

    if(tabState == LIST_TAB){
        for(i = 0; i < STAGE_NUM; i++){
            if (EffectList[i]){
                gwinSetText(label_effectName[i], EffectList[i]->name, 0);

            }else{
                gwinSetText(label_effectName[i], "", 0);
            }
            if(i == controllingStage){
                gwinSetText(btn_effectIndicate[i], "->", 0);
            }else{
                gwinSetText(btn_effectIndicate[i], "", 0);

            }
        }
    }else if(tabState == PARAM_TAB){
        if (EffectList[controllingStage]){
            strcpy(buf, "Stage ");
            itoa(controllingStage + 1, buf + 6);
            strcat(buf, " : ");
            strcat(buf, EffectList[controllingStage]->name);
            gwinSetText(label_effectTitle, buf, 1);

            EffectList[controllingStage]->getParam(EffectList[controllingStage], parameterList, &paraNum);
            for(i = 0; i < paraNum; i++){
                gwinSetVisible(label_param[i], TRUE);
                gwinSetText(label_param[i], parameterList[i]->name, 0);

                gwinSetVisible(vbar_param[i], TRUE);
                ftoa(parameterList[i]->value, buf, 2);
                gwinSetText(vbar_param[i], buf, 1);
                gwinProgressbarSetPosition(vbar_param[i], map(parameterList[i]->value, parameterList[i]->lowerBound, parameterList[i]->upperBound, 0, 100));
            }
            i = paraNum;
        }else{
            gwinSetText(label_effectTitle, "", 0);
        }
        for(; i < MAX_EFFECT_PARAM; i++){
            gwinSetVisible(vbar_param[i], FALSE);
            gwinSetVisible(label_param[i], FALSE);
        }
    }else if(tabState == SELECT_EFFECT_TAB){

    }
    return;
}

static void StageEffectSelect(uint8_t whichEffect)
{
    struct Effect_t *recycle = EffectList[controllingStage];

    //TODO: Implement FKING Factory
    switch(whichEffect){
    case VOL:
        EffectList[controllingStage] = new_Volume();
        break;
    case COMP:
        EffectList[controllingStage] = new_Compressor();
        break;
    case DISTOR:
        EffectList[controllingStage] = new_Distortion();
        break;
    case OVERDR:
        EffectList[controllingStage] = new_Overdrive();
        break;
    case DELAY:
        EffectList[controllingStage] = new_Delay();
        break;
    case REVERB:
        EffectList[controllingStage] = new_Reverb();
        break;
    case FLANGE:
        EffectList[controllingStage] = new_Flanger();
        break;
    case EQULIZ:
        EffectList[controllingStage] = new_Equalizer();
        break;
    default:
        EffectList[controllingStage] = NULL;
        break;
    }

    if(recycle){
        recycle->del(recycle);
    }

    /* Reset value in this stage */
    ValueForEachStage[controllingStage][0] = 0;
    ValueForEachStage[controllingStage][1] = 0;
    ValueForEachStage[controllingStage][2] = 0;

    if (EffectList[controllingStage]){
        EffectList[controllingStage]->adj(EffectList[controllingStage], ValueForEachStage[0]);
    }    
}

void SaveCurrentStageSetting(){
	FIL fil;
	char str[] = "jfjfjfjf";

	if (f_open(&fil, "0:/config0", FA_OPEN_ALWAYS | FA_READ) != FR_OK) for(;;);

	f_write(&fil, str, strlen(str), NULL);

	f_close(&fil);
}

void UserInterface(void *argument){
    GEvent* event;
    uint32_t i;
    uint32_t diff, cnt, orig;

    if (f_mount(&FatFs, SD_Path, 1) != FR_OK) for(;;);

    gfxInit();
    gdispClear(White);

    gwinSetDefaultFont(gdispOpenFont("DejaVuSans16"));
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    vTaskDelay(100);
    // Attach the mouse input
    gwinAttachMouse(0);

    // create the widget
    createWidgets(); 
    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    SwitchTab(LIST_TAB);

    SaveCurrentStageSetting();

    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)potValues[0], 3); //TODO: Make 4

    while(1) {
        // Get an Event
        event = geventEventWait(&gl, 20);

        switch(event->type) {
        case GEVENT_GWIN_BUTTON:
            for(i = 0; i < STAGE_NUM; i++){
                if (((GEventGWinButton*)event)->button == btn_effectIndicate[i]){
                    controllingStage = i;
                }
            }

            for(i = 0; i < STAGE_NUM; i++){
                if (((GEventGWinButton*)event)->button == btn_effectSwitch[i]){
                    controllingStage = i;
                    SwitchTab(SELECT_EFFECT_TAB);
                }
            }

            for(i = 0; i < EFFECT_TYPE_NUM - 1; i++){
                if (((GEventGWinButton*)event)->button == btn_effectTypes[i]){
                    StageEffectSelect(i);
                    SwitchTab(LIST_TAB);
                }
            }

            if (((GEventGWinButton*)event)->button == btn_effectTypes[EFFECT_TYPE_NUM -1]){
                SwitchTab(LIST_TAB);
            }
            break;
        }


        diff = 0;

        for(i = 0; i < 4; i++){
            if(((potValues[1][i] - potValues[0][i]) > 4) || ((potValues[0][i] - potValues[1][i]) > 4))
                diff++;
        }

        if(diff){
            if(EffectList[controllingStage]){
                EffectList[controllingStage]->adj(EffectList[controllingStage], potValues[0]);

                if(tabState == LIST_TAB){
                    orig = tabState;
                    SwitchTab(PARAM_TAB);
                }
                cnt = 0;
            }
        }

        for(i = 0; i < 4; i++){
            potValues[1][i] = potValues[0][i];
        }

        if(cnt == 75){
            SwitchTab(orig);
            cnt++;
        }else if(cnt < 75){
            cnt++;
        }

        RefreshScreen();
    }

    while(1){
    }
}
