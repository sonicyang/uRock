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

#include "base-effect.h"
#include "volume.h"
#include "delay.h"
#include "distortion.h"
#include "reverb.h"
#include "phaser.h"


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
struct Phaser_t phaser;

osThreadId LEDThread1Handle;
static void LED_Thread1(void const *argument);

osThreadId SPUid;
static void SignalProcessingUnit(void const *argument);
volatile uint32_t SPU_Hold = 0;
volatile uint16_t SignalBuffer[BUFFER_NUM][SAMPLE_NUM]; 
volatile float SignalPipe[STAGE_NUM][SAMPLE_NUM];
struct Effect_t *EffectStages[STAGE_NUM];

osThreadId UIid;
static void UserInterface(void const *argument);

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
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

	BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
	BSP_LCD_Clear(LCD_COLOR_RED);
	BSP_LCD_FillCircle(150, 150, 40);

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

/*
	osThreadDef(LED3, LED_Thread1, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	LEDThread1Handle = osThreadCreate (osThread(LED3), NULL);
*/
	osThreadDef(SPU, SignalProcessingUnit, osPriorityNormal, 0, 1024);
    SPUid = osThreadCreate (osThread(SPU), NULL);

	osThreadDef(UI, UserInterface, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    UIid = osThreadCreate (osThread(UI), NULL);

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

    //EffectStages[0] = new_Volume(&vol);
    //EffectStages[0] = new_Phaser(&phaser);
    //EffectStages[0] = new_Reverb(&delay);

    /* Init */
    HAL_TIM_Base_Start(&htim2);
    HAL_ADC_Start_DMA_DoubleBuffer(&hadc1, (uint32_t*)SignalBuffer[0], (uint32_t*)SignalBuffer[1], SAMPLE_NUM);
    HAL_DAC_Start_DMA_DoubleBuffer(&hdac, DAC_CHANNEL_2, (uint32_t*) SignalBuffer[1], (uint32_t*) SignalBuffer[2], SAMPLE_NUM, DAC_ALIGN_12B_R);
  
    /* Process */
    while(1){
        if(SPU_Hold){
            SPU_Hold--;

            NormalizeData(SignalBuffer[index], SignalPipe[pipeindex]);
            
            for(i = 0; i < STAGE_NUM; i++){
                if(EffectStages[i] != NULL){
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

static void UserInterface(void const *argument){
    uint8_t values[3];
    char buf[16];

	TS_StateTypeDef tp;
    uint32_t controllingStage = 0;

    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)values, 3);
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 0, (uint8_t*) "uROCK", CENTER_MODE);
    
    osDelay(10);
	while (1) {
		BSP_TS_GetState(&tp);
		if (tp.TouchDetected == 1) {
            controllingStage++;
            if(controllingStage >= 4)
                controllingStage = 0;
		}

        BSP_LCD_Clear(LCD_COLOR_WHITE);

        if(EffectStages[controllingStage] == NULL){
            BSP_LCD_DisplayStringAt(0, 0, (uint8_t*) "uROCK", CENTER_MODE);
        }else{
            BSP_LCD_DisplayStringAt(0, 0, (uint8_t*) "uROCK", CENTER_MODE);
            BSP_LCD_DisplayStringAt(0, 1 * 16, (uint8_t*) EffectStages[controllingStage]->name, CENTER_MODE);

            EffectStages[controllingStage]->adj(EffectStages[controllingStage], values);
        }

       /* 
        ftoa(EffectStages[controllingStage].parameter[0].value, buf, 2);
        BSP_LCD_DisplayStringAt(0, 3 * 16, (uint8_t*) buf, CENTER_MODE);
        ftoa(EffectStages[controllingStage].parameter[1].value, buf, 2);
        BSP_LCD_DisplayStringAt(0, 4 * 16, (uint8_t*) buf, CENTER_MODE);
        ftoa(EffectStages[controllingStage].parameter[2].value, buf, 2);
        BSP_LCD_DisplayStringAt(0, 5 * 16, (uint8_t*) buf, CENTER_MODE);
        */

        osDelay(200);
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
