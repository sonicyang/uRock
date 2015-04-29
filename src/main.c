#include "string.h"

#include "stm32f4xx_hal.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"
#include "stm32f4xx_it.h"

#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stm32f4xx_msp_init.h"
#include "ui.h"
#include "spu.h"
#include "setting.h"

//static void Error_Handler(void);
static void SystemClock_Config(void);

/* Private variables ---------------------------------------------------------*/
SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypedef SDCardInfo;
DMA_HandleTypeDef hdma_sdiorx;
DMA_HandleTypeDef hdma_sdiotx;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;

uint8_t SD_DriverNum;      /* FatFS SD part */
char SD_Path[4];           /* SD card logical drive path */
FATFS FatFs;

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

    MX_GPIO_Init();
    MX_SDIO_SD_Init();
    MX_DMA_Init();
    MX_TIM2_Init();
    MX_ADC1_Init();
    MX_ADC2_Init();
    MX_DAC_Init();
    MX_GPIO_Init();

    SD_DriverNum = FATFS_LinkDriver(&SD_Driver, SD_Path);

    xTaskCreate(SignalProcessingUnit,
        (signed char*)"SPU",
        1024, NULL, tskIDLE_PRIORITY + 3, NULL);

    xTaskCreate(UserInterface,
        (signed char*)"UI",
        2048, NULL, tskIDLE_PRIORITY + 1, NULL);

    vTaskStartScheduler();

    while (1);
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

/* SDIO init function */
void MX_SDIO_SD_Init(void)
{

    hsd.Instance = SDIO;
    hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv = 5;
    HAL_SD_Init(&hsd, &SDCardInfo);

    /* SDIO IRQ */
    HAL_NVIC_SetPriority(SDIO_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);

}

void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;
  ADC_MultiModeTypeDef multimode;
 
  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION12b;
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
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
 
  /**Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  multimode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_5CYCLES;
  HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode);
 
}
 
void MX_ADC2_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;
  ADC_MultiModeTypeDef multimode;
 
  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
  hadc2.Init.Resolution = ADC_RESOLUTION8b;
  hadc2.Init.ScanConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.NbrOfDiscConversion = 1;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 3;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DMAContinuousRequests = ENABLE;
  hadc2.Init.EOCSelection = EOC_SINGLE_CONV;
  HAL_ADC_Init(&hadc2);
 
  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  HAL_ADC_ConfigChannel(&hadc2, &sConfig);

  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 2;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  HAL_ADC_ConfigChannel(&hadc2, &sConfig);

  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 3;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  HAL_ADC_ConfigChannel(&hadc2, &sConfig);
 
  /**Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  multimode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_5CYCLES;
  HAL_ADCEx_MultiModeConfigChannel(&hadc2, &multimode);
 
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
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
  HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2);
 
}
 
/* TIM2 init function */
void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
 
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = (48000 / SAMPLING_RATE) - 1;
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

    /* ADC1 DMA IRQ */
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    /* DAC DMA IRQ */
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

    /* SDIO RX DMA IRQ */
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    /* SDIO TX DMA IRQ */
    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}
 
/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOH_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}
