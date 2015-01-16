#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "spu.h"
#include "ui.h"
#include "setting.h"
#include "base-effect.h"
#include "helper.h"

#include "tab_list.h"
#include "tab_param.h"
#include "tab_select_effect.h"

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

#include "wavplayer.h"

#include "cfgFunc.h"

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

struct tab_list_t listTab;
struct tab_param_t paramTab;
struct tab_select_effect_t selectEffectTab;

uint8_t currentConfig;

struct tab_t *tabs[TAB_NUM];
struct tab_t *current_tab = NULL;

static GListener gl;

static uint32_t tabState = 0;

uint8_t potValues[2][4];

void SwitchTab(uint32_t tab){
	if(current_tab)
		current_tab->hide(current_tab);

	current_tab = tabs[tab];
	current_tab->show(current_tab);

	tabState = tab;
	return;
}

void StageEffectSelect(uint8_t whichEffect){
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

void UserInterface(void *argument){
	GEvent* event;
	uint32_t i;
	uint32_t diff, cnt, orig;

	currentConfig = 0;

	if (f_mount(&FatFs, SD_Path, 1) != FR_OK) for(;;);

	gfxInit();
	gdispClear(White);

	gwinSetDefaultFont(gdispOpenFont("DejaVuSans16"));
	gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
	vTaskDelay(100);
	// Attach the mouse input
	gwinAttachMouse(0);

	// create the widget
	tabs[LIST_TAB] = tab_list_init(&listTab); 
	tabs[PARAM_TAB] = tab_param_init(&paramTab); 
	tabs[SELECT_EFFECT_TAB] = tab_select_effect_init(&selectEffectTab); 

	// We want to listen for widget events
	geventListenerInit(&gl);
	gwinAttachListener(&gl);

	SwitchTab(LIST_TAB);

	HAL_ADC_Start_DMA(&hadc2, (uint32_t*)potValues[0], 3); //TODO: Make 4

	ReadStageSetting(0);

	while(1) {
		// Get an Event
		event = geventEventWait(&gl, 50);

		switch(event->type) {
		case GEVENT_GWIN_BUTTON:
			current_tab->eHandle(current_tab, (GEventGWinButton*)event);
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
			SaveStageSetting(currentConfig);
			cnt++;
		}else if(cnt < 75){
			cnt++;
		}

		current_tab->refresh(current_tab);
	}

	while(1){
	}
}
