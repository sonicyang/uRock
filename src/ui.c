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

#define TAB_GROUP_1 0

DMA_HandleTypeDef hdma_adc2;
ADC_HandleTypeDef hadc2;

extern struct Effect_t *EffectList[EFFECT_NUM];
extern uint8_t ValueForEachStage[STAGE_NUM][MAX_EFFECT_PARAM];
extern int8_t controllingStage;

enum {
    NONE,
    VOL,
    DISTOR,
    OVERDR,
    FLANGE,
    EQULIZ,
    EFFECT_TYPE_NUM
};

static GHandle label_uRock;
static GHandle label_effectName;
static GHandle btn_prevStage;
static GHandle btn_nextStage;
static GHandle vbar_param[MAX_EFFECT_PARAM];
static GHandle btn_StageWidget;
static GHandle btn_SelectEffectWidget;
static GHandle btn_effectTypes[EFFECT_TYPE_NUM];

static char defaultName[8] = "";

static GListener gl;

static uint32_t tabState = 0;

static char *cvtToEffectName(uint32_t ee){
    switch(ee){
        case VOL:
            return "Volume";
        case DISTOR:
            return "Distortion";
        case OVERDR:
            return "OverDrive";
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

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = 60;
	wi.g.y = 30;
	wi.g.width = 120;
	wi.g.height = 20;
	wi.text = "";
	label_effectName = gwinLabelCreate(NULL, &wi);

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = 20;
	wi.g.width = 40;
	wi.g.height = 40;
	wi.text = "Prev";
	btn_prevStage = gwinButtonCreate(NULL, &wi);

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = (240 - 40 - 5);
	wi.g.y = 20;
	wi.g.width = 40;
	wi.g.height = 40;
	wi.text = "Next";
	btn_nextStage = gwinButtonCreate(NULL, &wi);

    for(i = 0; i < MAX_EFFECT_PARAM; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = TRUE;
        wi.g.x = 5;
        wi.g.y = 100 + i * 60;
        wi.g.width = (240 - 10);
        wi.g.height = 25;
        wi.text = defaultName;
        vbar_param[i] = gwinSliderCreate(NULL, &wi);
    }

    /* SelectStageTab */
    for(i = 0; i < EFFECT_TYPE_NUM; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = TRUE;
        wi.g.x = 5;
        wi.g.y = 5 + 35 * i;
        wi.g.width = 230;
        wi.g.height = 30;
        wi.text = cvtToEffectName(i);
        btn_effectTypes[i] = gwinButtonCreate(NULL, &wi);
    }

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.customDraw = gwinRadioDraw_Tab;
	wi.g.width = gdispGetWidth() / 2;
	wi.g.height = 50;
	wi.g.y = gdispGetHeight() - 15;
	wi.g.x = 0 * wi.g.width;
	wi.text = "Tab 1";
	btn_StageWidget = gwinRadioCreate(NULL, &wi, TAB_GROUP_1);

	wi.g.x = 1 * wi.g.width;
	wi.text = "Tab 2";
	btn_SelectEffectWidget = gwinRadioCreate(NULL, &wi, TAB_GROUP_1);
}

void SwitchTab(GHandle tab){
    uint32_t i;

	gwinSetVisible(label_uRock, FALSE);
	gwinSetVisible(label_effectName, FALSE);
	gwinSetVisible(btn_prevStage, FALSE);
	gwinSetVisible(btn_nextStage, FALSE);
    for(i = 0; i < MAX_EFFECT_PARAM; i++){
	    gwinSetVisible(vbar_param[i], FALSE);
    }
    for(i = 0; i < EFFECT_TYPE_NUM; i++){
	    gwinSetVisible(btn_effectTypes[i], FALSE);
    }

	if (tab == btn_StageWidget) {
		gwinSetVisible(label_uRock, TRUE);
    	gwinSetVisible(label_effectName, TRUE);
		gwinSetVisible(btn_prevStage, TRUE);
		gwinSetVisible(btn_nextStage, TRUE);
        for(i = 0; i < MAX_EFFECT_PARAM; i++){
            gwinSetVisible(vbar_param[i], TRUE);
        }

        tabState = 0;
	} else if (tab == btn_SelectEffectWidget) {
        for(i = 0; i < EFFECT_TYPE_NUM; i++){
            gwinSetVisible(btn_effectTypes[i], TRUE);
        }

        tabState = 1;
	}
}

static void RefreshScreen(void){
    struct parameter_t *parameterList[5];
    uint8_t paraNum;
    uint32_t i = 0;

    if(tabState == 0){
        if (EffectList[controllingStage]){
            gwinSetText(label_effectName, EffectList[controllingStage]->name, 0);

            EffectList[controllingStage]->getParam(EffectList[controllingStage], parameterList, &paraNum);
            for(; i < paraNum; i++){
                gwinSetVisible(vbar_param[i], TRUE);
                gwinSetText(vbar_param[i], parameterList[i]->name, 0);
            }
            i = paraNum;
        }else{
            gwinSetText(label_effectName, "", 0);
        }
        for(; i < MAX_EFFECT_PARAM; i++){
            gwinSetVisible(vbar_param[i], FALSE);
        }
    }else if(tabState == 1){

    }
    return;
}

static void StageSetValue(uint8_t whichParam, uint8_t value)
{
    ValueForEachStage[controllingStage][whichParam] = value;
    if (EffectList[controllingStage]){
        EffectList[controllingStage]->adj(
            EffectList[controllingStage],
            ValueForEachStage[controllingStage]);
    }
}

static void SelectNextStage()
{
    controllingStage++;

    if(controllingStage >= STAGE_NUM)
        controllingStage = 0;

    return;
}

static void SelectPrevStage()
{
    controllingStage--;

    if(controllingStage < 0)
        controllingStage = STAGE_NUM - 1;

    return;
}

static void StageEffectSelect(uint8_t whichEffect)
{
    struct Effect_t *recycle = EffectList[controllingStage];

    switch(whichEffect){
        case VOL:
            EffectList[controllingStage] = new_Volume();
            break;
        case DISTOR:
            EffectList[controllingStage] = new_Distortion();
            break;
        case OVERDR:
            EffectList[controllingStage] = new_Overdrive();
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

void UserInterface(void *argument){
	GEvent* event;
	char digits[4];
    uint32_t i;

	gfxInit();

	gdispClear(White);

	gwinSetDefaultFont(gdispOpenFont("fixed_7x14"));
	gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    
    vTaskDelay(100);
	// Attach the mouse input
	gwinAttachMouse(0);

	// create the widget
	createWidgets();

	// We want to listen for widget events
	geventListenerInit(&gl);
	gwinAttachListener(&gl);

    SwitchTab(btn_StageWidget);
    RefreshScreen();

	while(1) {
		// Get an Event
		event = geventEventWait(&gl, TIME_INFINITE);

		switch(event->type) {
		case GEVENT_GWIN_RADIO:
			switch(((GEventGWinRadio *)event)->group) {
			case TAB_GROUP_1:
				SwitchTab(((GEventGWinRadio *)event)->radio);
				break;
			default:
				break;
			}
			break;

		case GEVENT_GWIN_BUTTON:
			if (((GEventGWinButton*)event)->button == btn_prevStage)
				SelectPrevStage();
			else if (((GEventGWinButton*)event)->button == btn_nextStage)
				SelectNextStage();

            for(i = 0; i < EFFECT_TYPE_NUM; i++){
			    if (((GEventGWinButton*)event)->button == btn_effectTypes[i]){
    				StageEffectSelect(i);
                    SwitchTab(btn_StageWidget);
                }
            }
			break;

		case GEVENT_GWIN_SLIDER:
			itoa(((GEventGWinSlider *)event)->position, digits);
			gdispDrawString(0, 50, digits, gdispOpenFont("UI2"), HTML2COLOR(0xFF0000));
            
            for(i = 0; i < MAX_EFFECT_PARAM; i++){
                if (((GEventGWinSlider *)event)->slider == vbar_param[i]){
                    StageSetValue(i, map(((GEventGWinSlider *)event)->position, 0, 100, 0, 255));
                }
            }            
			break;
		}

        RefreshScreen();
	}
}
