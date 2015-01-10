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
#include "event.h"
#include "gui.h"

#define EFFECT_NUM 3
#define STAGE_NUM 4
DMA_HandleTypeDef hdma_adc2;
ADC_HandleTypeDef hadc2;

extern int8_t EffectStages[STAGE_NUM];
extern struct Effect_t *EffectList[EFFECT_NUM];
extern uint8_t ValueForEachStage[STAGE_NUM][3];
extern int8_t controllingStage;

enum Widget{
    WIDGET_STAGE = 0x00000000,
    WIDGET_PARAM
};
enum Widget widget = WIDGET_STAGE;

static void present(){
    static uint32_t currentDrawLayer = LCD_BACKGROUND_LAYER;

    if(currentDrawLayer == LCD_BACKGROUND_LAYER){
        BSP_LCD_SetTransparency(LCD_FOREGROUND_LAYER, 0);
        BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
        currentDrawLayer = LCD_FOREGROUND_LAYER;
    }else{
        BSP_LCD_SetTransparency(LCD_FOREGROUND_LAYER, 255);
        BSP_LCD_SelectLayer(LCD_BACKGROUND_LAYER);
        currentDrawLayer = LCD_BACKGROUND_LAYER;
    }
}

static void StageSetValue0(uint8_t value)
{
    ValueForEachStage[controllingStage][0] = value;
    /*if (EffectStages[controllingStage]){*/
        /*EffectStages[controllingStage]->adj(*/
            /*EffectStages[controllingStage],*/
            /*ValueForEachStage[controllingStage]);*/
    /*}*/
}

static void StageSetValue1(uint8_t value)
{
    ValueForEachStage[controllingStage][1] = value;
    /*if (EffectStages[controllingStage]){*/
        /*EffectStages[controllingStage]->adj(*/
            /*EffectStages[controllingStage],*/
            /*ValueForEachStage[controllingStage]);*/
    /*}*/
}

static void StageSetValue2(uint8_t value)
{
    ValueForEachStage[controllingStage][2] = value;
    /*if (EffectStages[controllingStage]){*/
        /*EffectStages[controllingStage]->adj(*/
            /*EffectStages[controllingStage],*/
            /*ValueForEachStage[controllingStage]);*/
    /*}*/
}

static uint8_t StageGetValue0()
{
    return ValueForEachStage[controllingStage][0];
}

static uint8_t StageGetValue1()
{
    return ValueForEachStage[controllingStage][1];
}

static uint8_t StageGetValue2()
{
    return ValueForEachStage[controllingStage][2];
}

static void SelectNextStage()
{
    controllingStage++;

    if(controllingStage == STAGE_NUM)
        controllingStage = 0;
}

static void SelectPrevStage()
{
    controllingStage--;

    if(controllingStage < 0)
        controllingStage = STAGE_NUM - 1;
}

static void SelectStageWidget()
{
    widget = WIDGET_STAGE;
}

static void Stage0Next()
{
    EffectStages[0]++;
    if (EffectStages[0] == EFFECT_NUM)
        EffectStages[0] = 0;

    /* Reset value in this stage */
    ValueForEachStage[0][0] = 0;
    ValueForEachStage[0][1] = 0;
    ValueForEachStage[0][2] = 0;

    if (EffectList[EffectStages[0]])
        EffectList[EffectStages[0]]->adj(EffectList[EffectStages[0]], ValueForEachStage[0]);
}

static void Stage1Next()
{
    EffectStages[1]++;
    if (EffectStages[1] == EFFECT_NUM)
        EffectStages[1] = 0;

    /* Reset value in this stage */
    ValueForEachStage[1][0] = 0;
    ValueForEachStage[1][1] = 0;
    ValueForEachStage[1][2] = 0;

    if (EffectList[EffectStages[1]])
        EffectList[EffectStages[1]]->adj(EffectList[EffectStages[1]], ValueForEachStage[1]);
}

static void Stage2Next()
{
    EffectStages[2]++;
    if (EffectStages[2] == EFFECT_NUM)
        EffectStages[2] = 0;

    /* Reset value in this stage */
    ValueForEachStage[2][0] = 0;
    ValueForEachStage[2][1] = 0;
    ValueForEachStage[2][2] = 0;

    if (EffectList[EffectStages[2]])
        EffectList[EffectStages[2]]->adj(EffectList[EffectStages[2]], ValueForEachStage[2]);
}

static void Stage3Next()
{
    EffectStages[3]++;
    if (EffectStages[3] == EFFECT_NUM)
        EffectStages[3] = 0;

    /* Reset value in this stage */
    ValueForEachStage[3][0] = 0;
    ValueForEachStage[3][1] = 0;
    ValueForEachStage[3][2] = 0;

    if (EffectList[EffectStages[3]])
        EffectList[EffectStages[3]]->adj(EffectList[EffectStages[3]], ValueForEachStage[3]);
}

static void SelectParamWidget()
{
    widget = WIDGET_PARAM;
}

static Button btn_prevStage;
static Button btn_nextStage;
static ValueBar vbar_param0;
static ValueBar vbar_param1;
static ValueBar vbar_param2;
static Button btn_StageWidget;
static Button btn_ParamWidget;
static Button btn_stage0;
static Button btn_stage1;
static Button btn_stage2;
static Button btn_stage3;

void UserInterface(void const *argument){
    Event event;
    TS_StateTypeDef tp;
    uint8_t lastTouchState = 0;

    char stageNum[3];

    /*HAL_ADC_Start_DMA(&hadc2, (uint32_t*)values, 3);*/

    BSP_LCD_SetTransparency(LCD_FOREGROUND_LAYER, 0);
    BSP_LCD_SetTransparency(LCD_BACKGROUND_LAYER, 255);
    BSP_LCD_SelectLayer(LCD_BACKGROUND_LAYER);

    /* Parameter widget */
    gui_ButtonInit(&btn_prevStage);
    gui_ButtonSetPos(&btn_prevStage, 5, 20);
    gui_ButtonSetSize(&btn_prevStage, 40, 40);
    gui_ButtonSetColor(&btn_prevStage, LCD_COLOR_BLUE);
    gui_ButtonSetRenderType(&btn_prevStage, BUTTON_RENDER_TYPE_LINE);
    gui_ButtonSetCallback(&btn_prevStage, SelectPrevStage);

    gui_ButtonInit(&btn_nextStage);
    gui_ButtonSetPos(&btn_nextStage, (240 - 40 - 5), 20);
    gui_ButtonSetSize(&btn_nextStage, 40, 40);
    gui_ButtonSetColor(&btn_nextStage, LCD_COLOR_BLUE);
    gui_ButtonSetRenderType(&btn_nextStage, BUTTON_RENDER_TYPE_LINE);
    gui_ButtonSetCallback(&btn_nextStage, SelectNextStage);

    gui_ValueBarInit(&vbar_param0);
    gui_ValueBarSetPos(&vbar_param0, 5, 100);
    gui_ValueBarSetSize(&vbar_param0, (240 - 10), 25);
    gui_ValueBarSetCallbacks(&vbar_param0, StageSetValue0, StageGetValue0);

    gui_ValueBarInit(&vbar_param1);
    gui_ValueBarSetPos(&vbar_param1, 5, 160);
    gui_ValueBarSetSize(&vbar_param1, (240 - 10), 25);
    gui_ValueBarSetCallbacks(&vbar_param1, StageSetValue1, StageGetValue1);

    gui_ValueBarInit(&vbar_param2);
    gui_ValueBarSetPos(&vbar_param2, 5, 220);
    gui_ValueBarSetSize(&vbar_param2, (240 - 10), 25);
    gui_ValueBarSetCallbacks(&vbar_param2, StageSetValue2, StageGetValue2);

    /* Stage widget */
    gui_ButtonInit(&btn_stage0);
    gui_ButtonSetPos(&btn_stage0, 5, 20);
    gui_ButtonSetSize(&btn_stage0, 40, 40);
    gui_ButtonSetColor(&btn_stage0, LCD_COLOR_RED);
    gui_ButtonSetRenderType(&btn_stage0, BUTTON_RENDER_TYPE_FILL);
    gui_ButtonSetCallback(&btn_stage0, Stage0Next);

    gui_ButtonInit(&btn_stage1);
    gui_ButtonSetPos(&btn_stage1, 5, 65);
    gui_ButtonSetSize(&btn_stage1, 40, 40);
    gui_ButtonSetColor(&btn_stage1, LCD_COLOR_RED);
    gui_ButtonSetRenderType(&btn_stage1, BUTTON_RENDER_TYPE_FILL);
    gui_ButtonSetCallback(&btn_stage1, Stage1Next);

    gui_ButtonInit(&btn_stage2);
    gui_ButtonSetPos(&btn_stage2, 5, 110);
    gui_ButtonSetSize(&btn_stage2, 40, 40);
    gui_ButtonSetColor(&btn_stage2, LCD_COLOR_RED);
    gui_ButtonSetRenderType(&btn_stage2, BUTTON_RENDER_TYPE_FILL);
    gui_ButtonSetCallback(&btn_stage2, Stage2Next);

    gui_ButtonInit(&btn_stage3);
    gui_ButtonSetPos(&btn_stage3, 5, 155);
    gui_ButtonSetSize(&btn_stage3, 40, 40);
    gui_ButtonSetColor(&btn_stage3, LCD_COLOR_RED);
    gui_ButtonSetRenderType(&btn_stage3, BUTTON_RENDER_TYPE_FILL);
    gui_ButtonSetCallback(&btn_stage3, Stage3Next);

    /* Global */
    gui_ButtonInit(&btn_StageWidget);
    gui_ButtonSetPos(&btn_StageWidget,  5, (320 - 10 - 5));
    gui_ButtonSetSize(&btn_StageWidget, ((240 - 5 * 3) / 2), 10);
    gui_ButtonSetColor(&btn_StageWidget, LCD_COLOR_BLACK);
    gui_ButtonSetRenderType(&btn_StageWidget, BUTTON_RENDER_TYPE_FILL);
    gui_ButtonSetCallback(&btn_StageWidget, SelectStageWidget);

    gui_ButtonInit(&btn_ParamWidget);
    gui_ButtonSetPos(&btn_ParamWidget, (240 - 5 - ((240 - 5 * 3) / 2)), (320 - 10 - 5));
    gui_ButtonSetSize(&btn_ParamWidget, ((240 - 5 * 3) / 2), 10);
    gui_ButtonSetColor(&btn_ParamWidget, LCD_COLOR_BLACK);
    gui_ButtonSetRenderType(&btn_ParamWidget, BUTTON_RENDER_TYPE_FILL);
    gui_ButtonSetCallback(&btn_ParamWidget, SelectParamWidget);

    while(1){
        while(1){
            BSP_TS_GetState(&tp);
            if(tp.TouchDetected != lastTouchState){
                if(tp.TouchDetected == 1){
                    lastTouchState = 1;
                    event.eventType = TP_PRESSED;
                    event.touchX = tp.X;
                    event.touchY = tp.Y;
                }else if(tp.TouchDetected == 0){
                    lastTouchState = 0;
                    event.eventType = TP_RELEASED;
                    event.touchX = tp.X;
                    event.touchY = tp.Y;
                }
                break;
            }
            vTaskDelay(50);
        }

        switch(widget){
        case WIDGET_STAGE:
            /* Event handle & update part */
            gui_ButtonHandleEvent(&btn_nextStage, &event);
            gui_ButtonHandleEvent(&btn_prevStage, &event);

            gui_ValueBarHandleEvent(&vbar_param0, &event);
            gui_ValueBarHandleEvent(&vbar_param1, &event);
            gui_ValueBarHandleEvent(&vbar_param2, &event);

            /* Render part */
            BSP_LCD_Clear(LCD_COLOR_WHITE);

            BSP_LCD_DisplayStringAt(0, 0, (uint8_t*) "uROCK", CENTER_MODE);
            intToStr(controllingStage, stageNum, 2);
            BSP_LCD_DisplayStringAt(0, 20, (uint8_t*) stageNum, CENTER_MODE);

            if(EffectStages[controllingStage])
                BSP_LCD_DisplayStringAt(0, 40, (uint8_t*) EffectList[EffectStages[controllingStage]]->name, CENTER_MODE);
            else
                BSP_LCD_DisplayStringAt(0, 40, (uint8_t*) "< None >", CENTER_MODE);

            gui_ButtonRender(&btn_nextStage);
            gui_ButtonRender(&btn_prevStage);

            gui_ValueBarRender(&vbar_param0);
            gui_ValueBarRender(&vbar_param1);
            gui_ValueBarRender(&vbar_param2);
            break;
        case WIDGET_PARAM:
            gui_ButtonHandleEvent(&btn_stage0, &event);
            gui_ButtonHandleEvent(&btn_stage1, &event);
            gui_ButtonHandleEvent(&btn_stage2, &event);
            gui_ButtonHandleEvent(&btn_stage3, &event);

            BSP_LCD_Clear(LCD_COLOR_YELLOW);

            for(int8_t i = 0; i < STAGE_NUM; i++){
                if(EffectStages[i])
                    BSP_LCD_DisplayStringAt(0, 30 + 45 * i,(uint8_t*) EffectList[EffectStages[i]]->name, CENTER_MODE);
                else
                    BSP_LCD_DisplayStringAt(0, 30 + 45 * i, (uint8_t*) "< NONE >", CENTER_MODE);
            }

            gui_ButtonRender(&btn_stage0);
            gui_ButtonRender(&btn_stage1);
            gui_ButtonRender(&btn_stage2);
            gui_ButtonRender(&btn_stage3);
            break;
        }

        gui_ButtonHandleEvent(&btn_StageWidget, &event);
        gui_ButtonHandleEvent(&btn_ParamWidget, &event);

        gui_ButtonRender(&btn_StageWidget);
        gui_ButtonRender(&btn_ParamWidget);

        present();
    }
}
