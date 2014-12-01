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
#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery.h"

//static void Error_Handler(void);
static void SystemClock_Config(void);

/* Private variables ---------------------------------------------------------*/
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_dac2;
ADC_HandleTypeDef hadc1;
DAC_HandleTypeDef hdac;
TIM_HandleTypeDef htim2;

volatile uint8_t value[8][256]; 
volatile uint8_t value2[8] = {255, 255, 255, 255, 0, 0, 0, 0}; 
volatile uint8_t buffer_index = 0;

/* USER CODE BEGIN 0 */
 
/* ADC1 init function */
void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;
  ADC_MultiModeTypeDef multimode;
 
  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION8b;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = EOC_SINGLE_CONV;
  HAL_ADC_Init(&hadc1);
 
  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
 
  /**Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  multimode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_5CYCLES;
  HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode);
 
}
 
/* DAC init function */
void MX_DAC_Init(void)
{
  DAC_ChannelConfTypeDef sConfig;
 
  /**DAC Initialization
  */
  hdac.Instance = DAC;
  HAL_DAC_Init(&hdac);
 
  /**DAC channel OUT2 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2);
 
}
 
/* TIM2 init function */
void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
 
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 100;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim2);
 
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
 
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
 
}
 
/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void) {
  /* DMA controller clock enable */
  __DMA2_CLK_ENABLE();
  __DMA1_CLK_ENABLE();
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}
 
/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void) {
 
  /* GPIO Ports Clock Enable */
  __GPIOH_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
}
/* USER CODE END 0 */

void DMA2_Stream0_IRQHandler(void){
    HAL_DMA_IRQHandler(&hdma_adc1);
    return;
}

void DMA1_Stream6_IRQHandler(void){
    HAL_DMA_IRQHandler(&hdma_dac2);
    return;
}


int main(void){
	/* STM32F4xx HAL library initialization:
	   - Configure the Flash prefetch, instruction and Data caches
	   - Configure the Systick to generate an interrupt each 1 msec
	   - Set NVIC Group Priority to 4
	   - Global MSP (MCU Support Package) initialization
	 */
	HAL_Init();  

	/* Initialize LEDs */
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
    BSP_LED_On(LED3);
    BSP_LED_On(LED4);

	/* Configure the system clock to 180 Mhz */
	SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM2_Init();
    MX_ADC1_Init();
    MX_DAC_Init();

    HAL_TIM_Base_Start(&htim2);
    HAL_ADC_Start_DMA_DoubleBuffer(&hadc1, (uint32_t*)value[0], (uint32_t*)value[1], 256);
    HAL_DAC_Start_DMA_DoubleBuffer(&hdac, DAC_CHANNEL_2, (uint32_t*) value[1], (uint32_t*) value[2], 256, DAC_ALIGN_8B_R);
    //HAL_ADC_Start_DMA(&hadc1, (uint32_t*)value[0], 256);
    //HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) value[0], 256, DAC_ALIGN_8B_R);

	/* We should never get here as control is now taken by the scheduler */
	for(;;);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    static uint32_t index = 0;
    uint32_t i = 0;
    uint32_t oindex;

    oindex = index;
    index += 2;
    if(index > 7)
        index = 0;

    HAL_DMAEx_ChangeMemory(&hdma_adc1, (uint32_t)value[index], MEMORY0);

    
    for(; i < 256; i++){
        value[oindex][i] <<= 1;
    }
    

    return;  
}

void HAL_ADC_ConvM1CpltCallback(ADC_HandleTypeDef* hadc){
    static uint32_t index = 1;
    uint32_t i = 0;
    uint32_t oindex;

    oindex = index;
    index += 2;
    if(index > 7)
        index = 1;

    HAL_DMAEx_ChangeMemory(&hdma_adc1, (uint32_t)value[index], MEMORY1);

    
    for(; i < 256; i++){
        value[oindex][i] <<= 1;
    }
    

    return;  
}

void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac){
    static uint32_t index = 1;

    index += 2;
    if(index > 7)
        index = 1;

    HAL_DMAEx_ChangeMemory(&hdma_dac2, (uint32_t)value[index], MEMORY0);

    return;  
}

void HAL_DACEx_ConvM1CpltCallbackCh2(DAC_HandleTypeDef* hdac){
    static uint32_t index = 2;

    index += 2;
    if(index > 7)
        index = 0;

    HAL_DMAEx_ChangeMemory(&hdma_dac2, (uint32_t)value[index], MEMORY1);

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
	 clocked below the maximum system frequency, to update the voltage scaling value 
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
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1);
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
