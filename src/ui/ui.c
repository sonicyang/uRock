#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"
#include "stm32f4xx_hal_gpio.h"

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

#include "volume.h"
#include "compressor.h"

#include "distortion.h"
#include "overdrive.h"

#include "equalizer.h"

#include "delay.h"
#include "reverb.h"

#include "phaser.h"
#include "flanger.h"

//#include "wavplayer.h"

#include "cfgFunc.h"

//#include "ff.h"

#define TAB_GROUP_1 0

//extern char SD_Path[4];
//extern FATFS FatFs;
//extern FIL fil;

extern ADC_HandleTypeDef hadc2;
extern uint8_t ValueForEachStage[STAGE_NUM][MAX_EFFECT_PARAM];

struct tab_t *tabs[TAB_NUM];
static uint32_t currentTabNumber = 0;

struct tab_list_t listTab;
struct tab_param_t paramTab;
struct tab_select_effect_t selectEffectTab;

uint32_t selectedEffectStage = 0;

uint8_t currentConfig;


GPIO_PinState buttonPrevValue[MAX_CONFIG_NUM];



/*
// Reset value in this stage 
	ValueForEachStage[controllingStage][0] = 0;
	ValueForEachStage[controllingStage][1] = 0;
	ValueForEachStage[controllingStage][2] = 0;

	if (EffectList[controllingStage]){
		EffectList[controllingStage]->adj(EffectList[controllingStage], ValueForEachStage[0]);
	}
}
*/

void UserInterface(void const *argument){
	uint32_t i;
	uint32_t diff_flag, cnt, orig;

    static GListener gl;
	GEvent* event;

    uint8_t potValues[2][4];
    uint8_t potApply[4];

	currentConfig = 0;

    /*
    buttonPrevValue[0] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2);
    buttonPrevValue[1] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3);
    buttonPrevValue[2] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4);
    buttonPrevValue[3] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_5);
    */

	//if (f_mount(&FatFs, SD_Path, 1) != FR_OK) for(;;);

	gfxInit();
	gdispClear(Black);

	gwinSetDefaultFont(gdispOpenFont("DejaVuSans20"));
	gwinSetDefaultStyle(&BlackWidgetStyle, FALSE);
    geventListenerInit(&gl);
    gwinAttachListener(&gl);
    
	// create the widget
	tabs[LIST_TAB] = tab_list_init(&listTab); 
	tabs[PARAM_TAB] = tab_param_init(&paramTab); 
	tabs[SELECT_EFFECT_TAB] = tab_select_effect_init(&selectEffectTab); 
	SwitchTab(LIST_TAB);

	HAL_ADC_Start_DMA(&hadc2, (uint32_t*)potValues[0], 3); //TODO: Make 4

	//ReadStageSetting(0);

	while(1) {
		// Get an Event
		event = geventEventWait(&gl, 50);

		switch(event->type) {
            case GEVENT_GWIN_BUTTON:
                tabs[currentTabNumber]->bHandle(tabs[currentTabNumber], (GEventGWinButton*)event);
                break;
		}


        //XXX: Use Event System and Callback
        //TODO: Sync Pots before set new value
        if(currentTabNumber == PARAM_TAB){
            diff_flag = 0;

            for(i = 0; i < 4; i++){
                if(((potValues[1][i] - potValues[0][i]) > 5) || ((potValues[0][i] - potValues[1][i]) > 5)){
                    potApply[i] = potValues[0][i];
                    diff_flag++;
                }else{
                   potApply[i] = potValues[1][i];
                }
            }

            if(diff_flag){
                if (retriveStagedEffect(selectedEffectStage))
                    retriveStagedEffect(selectedEffectStage)->adj((void*)retriveStagedEffect(selectedEffectStage), potApply);

                for(i = 0; i < 4; i++){
                    potValues[1][i] = potApply[i];
                }
                tabs[currentTabNumber]->refresh(tabs[currentTabNumber]);
            }
        }
        
        /*
        if(tabState == LIST_TAB){
            if (buttonPrevValue[0] != HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2)){
                buttonPrevValue[0] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2);
                SaveStageSetting(currentConfig);
                currentConfig = 0;
                ReadStageSetting(currentConfig);
            }
            if (buttonPrevValue[1] != HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)){
                buttonPrevValue[1] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3);
                SaveStageSetting(currentConfig);
                currentConfig = 1;
                ReadStageSetting(currentConfig);
            }
            if (buttonPrevValue[2] != HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)){
                buttonPrevValue[2] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4);
                SaveStageSetting(currentConfig);
                currentConfig = 2;
                ReadStageSetting(currentConfig);
            }
            if (buttonPrevValue[3] != HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_5)){
                buttonPrevValue[3] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_5);
                SaveStageSetting(currentConfig);
                currentConfig = 3;
                ReadStageSetting(currentConfig);
            }
        }
        */
	}

	while(1);
}

void SwitchTab(uint32_t tabSwitching){
	tabs[currentTabNumber]->hide(tabs[currentTabNumber]);
	currentTabNumber = tabSwitching;
	tabs[currentTabNumber]->show(tabs[currentTabNumber]);
    tabs[currentTabNumber]->refresh(tabs[currentTabNumber]);

	return;
}
