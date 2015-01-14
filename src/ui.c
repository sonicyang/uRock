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

#define TAB_GROUP_1 0

DMA_HandleTypeDef hdma_adc2;
ADC_HandleTypeDef hadc2;

extern int8_t EffectStages[STAGE_NUM];
extern struct Effect_t *EffectList[EFFECT_NUM];
extern uint8_t ValueForEachStage[STAGE_NUM][3];
extern int8_t controllingStage;

static GHandle label_uRock;
static GHandle btn_prevStage;
static GHandle btn_nextStage;
static GHandle vbar_param0;
static GHandle vbar_param1;
static GHandle vbar_param2;
static GHandle btn_StageWidget;
static GHandle btn_ParamWidget;
static GHandle btn_stage0;
static GHandle btn_stage1;
static GHandle btn_stage2;
static GHandle btn_stage3;

static char defaultName[8] = "";

static char *parameterNameHook[3] = {defaultName};

static GListener gl;

static void createWidgets(void) {
	GWidgetInit wi;

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

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = (240 - 40 - 5);
	wi.g.y = 20;
	wi.g.width = 40;
	wi.g.height = 40;
	wi.text = "Next";
	btn_nextStage = gwinButtonCreate(NULL, &wi);

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = 100;
	wi.g.width = (240 - 10);
	wi.g.height = 25;
	wi.text = parameterNameHook[0];
	vbar_param0 = gwinSliderCreate(NULL, &wi);

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = 160;
	wi.g.width = (240 - 10);
	wi.g.height = 25;
	wi.text = parameterNameHook[1];
	vbar_param1 = gwinSliderCreate(NULL, &wi);

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = 220;
	wi.g.width = (240 - 10);
	wi.g.height = 25;
	wi.text = parameterNameHook[2];
	vbar_param2 = gwinSliderCreate(NULL, &wi);

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = 20;
	wi.g.width = 40;
	wi.g.height = 40;
	wi.text = "Next";
	btn_stage0 = gwinButtonCreate(NULL, &wi);

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = 65;
	wi.g.width = 40;
	wi.g.height = 40;
	wi.text = "Next";
	btn_stage1 = gwinButtonCreate(NULL, &wi);

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = 110;
	wi.g.width = 40;
	wi.g.height = 40;
	wi.text = "Next";
	btn_stage2 = gwinButtonCreate(NULL, &wi);

	gwinWidgetClearInit(&wi);
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = 155;
	wi.g.width = 40;
	wi.g.height = 40;
	wi.text = "Next";
	btn_stage3 = gwinButtonCreate(NULL, &wi);

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
	btn_ParamWidget = gwinRadioCreate(NULL, &wi, TAB_GROUP_1);
}

void SwitchTab(GHandle tab){
	gwinSetVisible(label_uRock, FALSE);
	gwinSetVisible(btn_prevStage, FALSE);
	gwinSetVisible(btn_nextStage, FALSE);
	gwinSetVisible(vbar_param0, FALSE);
	gwinSetVisible(vbar_param1, FALSE);
	gwinSetVisible(vbar_param2, FALSE);
	gwinSetVisible(btn_stage0, FALSE);
	gwinSetVisible(btn_stage1, FALSE);
	gwinSetVisible(btn_stage2, FALSE);
	gwinSetVisible(btn_stage3, FALSE);

	if (tab == btn_StageWidget) {
		gwinSetVisible(label_uRock, TRUE);
		gwinSetVisible(btn_prevStage, TRUE);
		gwinSetVisible(btn_nextStage, TRUE);
		gwinSetVisible(vbar_param0, TRUE);
		gwinSetVisible(vbar_param1, TRUE);
		gwinSetVisible(vbar_param2, TRUE);
	} else if (tab == btn_ParamWidget) {
		gwinSetVisible(btn_stage0, TRUE);
		gwinSetVisible(btn_stage1, TRUE);
		gwinSetVisible(btn_stage2, TRUE);
		gwinSetVisible(btn_stage3, TRUE);
	}
}

static void StageSetValue(uint8_t whichParam, uint8_t value)
{
    ValueForEachStage[controllingStage][whichParam] = value;
    if (EffectList[EffectStages[controllingStage]]){
        EffectList[EffectStages[controllingStage]]->adj(
            EffectList[EffectStages[controllingStage]],
            ValueForEachStage[controllingStage]);
    }
}

static void SelectNextStage()
{
    struct parameter_t *parameterList[5];
    uint8_t paraNum;
    uint32_t i;

    controllingStage++;

    if(controllingStage >= STAGE_NUM)
        controllingStage = 0;

    if (EffectList[EffectStages[controllingStage]]){
        EffectList[EffectStages[controllingStage]]->getParam(EffectList[EffectStages[0]], parameterList, &paraNum);
        for(i = 0; i < paraNum; i++){
            parameterNameHook[i] = parameterList[i]->name;
        }
        for(i = paraNum; i < 3; i++){
            parameterNameHook[i] = defaultName;
        }
    }else{
        for(i = 0; i < 3; i++){
            parameterNameHook[i] = defaultName;    
        }
    }    
}

static void SelectPrevStage()
{
    controllingStage--;

    if(controllingStage < 0)
        controllingStage = STAGE_NUM - 1;

    //TODO: make a refresh Function
    if (EffectList[EffectStages[controllingStage]]){
        EffectList[EffectStages[controllingStage]]->getParam(EffectList[EffectStages[0]], parameterList, &paraNum);
        for(i = 0; i < paraNum; i++){
            parameterNameHook[i] = parameterList[i]->name;
        }
        for(i = paraNum; i < 3; i++){
            parameterNameHook[i] = defaultName;
        }
    }else{
        for(i = 0; i < 3; i++){
            parameterNameHook[i] = defaultName;    
        }
    }    
}

static void StageEffectNext(uint8_t whichStage)
{

    EffectStages[whichStage]++;
    if (EffectStages[whichStage] == EFFECT_NUM)
        EffectStages[whichStage] = 0;

    //TODO: WHY Reset?
    /* Reset value in this stage */
    ValueForEachStage[whichStage][0] = 0;
    ValueForEachStage[whichStage][1] = 0;
    ValueForEachStage[whichStage][2] = 0;

    if (EffectList[EffectStages[whichStage]]){
        EffectList[EffectStages[whichStage]]->adj(EffectList[EffectStages[0]], ValueForEachStage[0]);
    }    
}

void UserInterface(void *argument){
<<<<<<< HEAD
	GEvent* event;
	char digits[4];

	gfxInit();

	gdispClear(White);

	gwinSetDefaultFont(gdispOpenFont("fixed_7x14"));
	gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);

	// Attach the mouse input
	gwinAttachMouse(0);

	// create the widget
	createWidgets();

	// We want to listen for widget events
	geventListenerInit(&gl);
	gwinAttachListener(&gl);

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
				SelectNextStage();
			else if (((GEventGWinButton*)event)->button == btn_nextStage)
				SelectPrevStage();
			else if (((GEventGWinButton*)event)->button == btn_stage0)
				StageEffectNext(0);
			else if (((GEventGWinButton*)event)->button == btn_stage1)
				StageEffectNext(1);
			else if (((GEventGWinButton*)event)->button == btn_stage2)
				StageEffectNext(2);
			else if (((GEventGWinButton*)event)->button == btn_stage3)
				StageEffectNext(3);
			break;

		case GEVENT_GWIN_SLIDER:
			itoa(((GEventGWinSlider *)event)->position, digits);
			gdispDrawString(0, 50, digits, gdispOpenFont("UI2"), HTML2COLOR(0xFF0000));

			if (((GEventGWinSlider *)event)->slider == vbar_param0){
				StageSetValue(0, map(((GEventGWinSlider *)event)->position, 0, 100, 0, 255));
            }else if (((GEventGWinSlider *)event)->slider == vbar_param1){
				StageSetValue(1, map(((GEventGWinSlider *)event)->position, 0, 100, 0, 255));
            }else if (((GEventGWinSlider *)event)->slider == vbar_param2){
				StageSetValue(2, map(((GEventGWinSlider *)event)->position, 0, 100, 0, 255));
            }
			break;
		}
	}
}
