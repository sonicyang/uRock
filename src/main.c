/**
 ******************************************************************************
 * @file    FreeRTOS/FreeRTOS_ThreadCreation/Src/main.c
 * @author  MCD Application Team
 * @version V1.1.0
 * @date    26-June-2014
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "string.h"

#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#include "cmsis_os.h"

#include "MspInit.h"
#include "helper.h"
#include "setting.h"

#include "gui.h"
#include "event.h"

#include "base-effect.h"
#include "volume.h"
#include "delay.h"
#include "distortion.h"
#include "reverb.h"


//static void Error_Handler(void);
static void SystemClock_Config(void);

/* Private variables ---------------------------------------------------------*/
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_dac2;
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DAC_HandleTypeDef hdac;
TIM_HandleTypeDef htim2;

struct Volume_t vol;
struct Distortion_t distor;
struct Reverb_t delay;

osThreadId LEDThread1Handle;
static void LED_Thread1(void const *argument);

osThreadId SPUid;
static void SignalProcessingUnit(void const *argument);
volatile uint32_t SPU_Hold = 0;
volatile uint16_t SignalBuffer[BUFFER_NUM][SAMPLE_NUM];
volatile float SignalPipe[STAGE_NUM][SAMPLE_NUM];
struct Effect_t *EffectStages[STAGE_NUM];
struct Effect_t *EffectList[3];
uint8_t valueForEachStage[STAGE_NUM][3];

osThreadId UIid;
static void UserInterface(void const *argument);

osThreadId InputEventId;
static void InputEvent(void const *argument);

Event event;
volatile int8_t controllingStage = 0;

enum Widget{
    WIDGET_STAGE = 0x00000000,
    WIDGET_PARAM
};
enum Widget widget = WIDGET_STAGE;

int main(void){
    /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
       */

    /* Configure the system clock to 180 Mhz */
    SystemClock_Config();

    HAL_Init();

    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, LCD_FRAME_BUFFER);
    BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER, LCD_FRAME_BUFFER + BUFFER_OFFSET);

    BSP_LCD_SelectLayer(LCD_BACKGROUND_LAYER);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font20);

    BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font20);

    BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

    BSP_SDRAM_Init();

    BSP_LED_Init(LED3);
    BSP_LED_Init(LED4);

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM2_Init();
    MX_ADC1_Init();
    MX_ADC2_Init();
    MX_DAC_Init();

    osThreadDef(LED3, LED_Thread1, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    LEDThread1Handle = osThreadCreate (osThread(LED3), NULL);

    osThreadDef(SPU, SignalProcessingUnit, osPriorityNormal, 0, 1024);
    SPUid = osThreadCreate (osThread(SPU), NULL);

    osThreadDef(UI, UserInterface, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    UIid = osThreadCreate (osThread(UI), NULL);

    osThreadDef(IE, InputEvent, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    InputEventId = osThreadCreate (osThread(IE), NULL);

    osKernelStart (NULL, NULL);

    while (1);
}

static void SignalProcessingUnit(void const *argument){
    uint32_t index = 0;
    uint32_t pipeindex = 0;
    uint32_t i;

    for(i = 0; i < STAGE_NUM; i++){
        EffectStages[i] = NULL;
    }

    /* Effect Stage Setting*/

    EffectList[0] = new_Volume(&vol);
    EffectList[1] = new_Distortion(&distor);
    EffectList[2] = new_Reverb(&delay);

    EffectStages[0] = EffectList[0];
    EffectStages[1] = EffectList[1];
    EffectStages[2] = EffectList[2];
    EffectStages[3] = EffectList[0];

    /* Init */
    HAL_TIM_Base_Start(&htim2);
    HAL_ADC_Start_DMA_DoubleBuffer(&hadc1, (uint32_t*)SignalBuffer[0], (uint32_t*)SignalBuffer[1], SAMPLE_NUM);
    HAL_DAC_Start_DMA_DoubleBuffer(&hdac, DAC_CHANNEL_2, (uint32_t*) SignalBuffer[1], (uint32_t*) SignalBuffer[2], SAMPLE_NUM, DAC_ALIGN_12B_R);

    /* Process */
    while(1){
        if(SPU_Hold){
            SPU_Hold--;

            for(i = 0; i < STAGE_NUM; i++){
                if(EffectStages[i]){
                    EffectStages[i]->func(SignalPipe[(pipeindex - i) % STAGE_NUM], EffectStages[i]);
                }
            }

            DenormalizeData(SignalPipe[(pipeindex - STAGE_NUM + 1) % STAGE_NUM], SignalBuffer[(index - 1) % BUFFER_NUM]);

            index+=1;
            if(index >= BUFFER_NUM)
                index = 0;
            pipeindex+=1;
            if(pipeindex >= STAGE_NUM)
                pipeindex = 0;
        }
    }

    while(1);
}

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
    valueForEachStage[controllingStage][0] = value;
    if (EffectStages[controllingStage]){
        EffectStages[controllingStage]->adj(
            EffectStages[controllingStage],
            valueForEachStage[controllingStage]);
    }
}

static void StageSetValue1(uint8_t value)
{
    valueForEachStage[controllingStage][1] = value;
    if (EffectStages[controllingStage]){
        EffectStages[controllingStage]->adj(
            EffectStages[controllingStage],
            valueForEachStage[controllingStage]);
    }
}

static void StageSetValue2(uint8_t value)
{
    valueForEachStage[controllingStage][2] = value;
    if (EffectStages[controllingStage]){
        EffectStages[controllingStage]->adj(
            EffectStages[controllingStage],
            valueForEachStage[controllingStage]);
    }
}

static uint8_t StageGetValue0()
{
    return valueForEachStage[controllingStage][0];
}

static uint8_t StageGetValue1()
{
    return valueForEachStage[controllingStage][1];
}

static uint8_t StageGetValue2()
{
    return valueForEachStage[controllingStage][2];
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

static void stage0Next()
{
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
static void UserInterface(void const *argument){
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
    gui_ButtonSetCallback(&btn_stage0, SelectPrevStage);

    gui_ButtonInit(&btn_stage1);
    gui_ButtonSetPos(&btn_stage1, 5, 65);
    gui_ButtonSetSize(&btn_stage1, 40, 40);
    gui_ButtonSetColor(&btn_stage1, LCD_COLOR_RED);
    gui_ButtonSetRenderType(&btn_stage1, BUTTON_RENDER_TYPE_FILL);
    gui_ButtonSetCallback(&btn_stage1, SelectPrevStage);

    gui_ButtonInit(&btn_stage2);
    gui_ButtonSetPos(&btn_stage2, 5, 110);
    gui_ButtonSetSize(&btn_stage2, 40, 40);
    gui_ButtonSetColor(&btn_stage2, LCD_COLOR_RED);
    gui_ButtonSetRenderType(&btn_stage2, BUTTON_RENDER_TYPE_FILL);
    gui_ButtonSetCallback(&btn_stage2, SelectPrevStage);

    gui_ButtonInit(&btn_stage3);
    gui_ButtonSetPos(&btn_stage3, 5, 155);
    gui_ButtonSetSize(&btn_stage3, 40, 40);
    gui_ButtonSetColor(&btn_stage3, LCD_COLOR_RED);
    gui_ButtonSetRenderType(&btn_stage3, BUTTON_RENDER_TYPE_FILL);
    gui_ButtonSetCallback(&btn_stage3, SelectPrevStage);

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

            BSP_LCD_DisplayStringAt(5, 0, (uint8_t*) "uROCK", CENTER_MODE);

            if(EffectStages[controllingStage])
                BSP_LCD_DisplayStringAt(0, 30, (uint8_t*) EffectStages[controllingStage]->name, CENTER_MODE);
            else
                BSP_LCD_DisplayStringAt(0, 30, (uint8_t*) "< None >", CENTER_MODE);

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
                    BSP_LCD_DisplayStringAt(0, 30 + 45 * i,(uint8_t*) EffectStages[i]->name, CENTER_MODE);
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

        osThreadResume(InputEventId);
        osThreadSuspend(UIid);
    }
}

static void InputEvent(void const *argument){
    TS_StateTypeDef tp;
    uint8_t lastTouchState = 0;

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

            osThreadResume(UIid);
            osThreadSuspend(InputEventId);
        }

        osDelay(50);
    }
}

static void LED_Thread1(void const *argument){
    (void) argument;

    for(;;){
        osDelay(300);
        BSP_LED_Toggle(LED3);
    }
}

/* Double Buffer Swapping Callbacks */
void DMA2_Stream0_IRQHandler(void){
    HAL_DMA_IRQHandler(&hdma_adc1);
    return;
}

void DMA1_Stream6_IRQHandler(void){
    HAL_DMA_IRQHandler(&hdma_dac2);
    return;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    static uint32_t index = 0;
    if(hadc->Instance == ADC1){
        index += 2;
        if(index >= BUFFER_NUM)
            index = 0;

        HAL_DMAEx_ChangeMemory(&hdma_adc1, (uint32_t)SignalBuffer[index], MEMORY0);

        SPU_Hold++;
    }
    return;
}

void HAL_ADC_ConvM1CpltCallback(ADC_HandleTypeDef* hadc){
    static uint32_t index = 1;

    if(hadc->Instance == ADC1){
        index += 2;
        if(index >= BUFFER_NUM)
            index = 1;

        HAL_DMAEx_ChangeMemory(&hdma_adc1, (uint32_t)SignalBuffer[index], MEMORY1);

        SPU_Hold++;
    }
    return;
}

void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac){
    static uint32_t index = 1;

    index += 2;
    if(index >= BUFFER_NUM)
        index = 1;

    HAL_DMAEx_ChangeMemory(&hdma_dac2, (uint32_t)SignalBuffer[index], MEMORY0);

    return;
}

void HAL_DACEx_ConvM1CpltCallbackCh2(DAC_HandleTypeDef* hdac){
    static uint32_t index = 2;

    index += 2;
    if(index >= BUFFER_NUM)
        index = 0;

    HAL_DMAEx_ChangeMemory(&hdma_dac2, (uint32_t)SignalBuffer[index], MEMORY1);

    return;
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 180000000
 *            HCLK(Hz)                       = 180000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 8000000
 *            PLL_M                          = 8
 *            PLL_N                          = 360
 *            PLL_P                          = 2
 *            PLL_Q                          = 7
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 5
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void){
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable Power Control clock */
    __PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling SignalPipe
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Activate the Over-Drive mode */
    HAL_PWREx_ActivateOverDrive();

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();

    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
    __HAL_FLASH_DATA_CACHE_ENABLE();
}


/*
static void Error_Handler(void){
    BSP_LED_On(LED4);
    while(1);
}
*/

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *   where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line){
    /* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters SignalPipe: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1);
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
