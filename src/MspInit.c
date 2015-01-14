#include "FreeRTOS.h"
#include "MspInit.h"
#include "setting.h"

extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern DMA_HandleTypeDef hdma_dac2;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;

extern SD_HandleTypeDef hsd;
extern HAL_SD_CardInfoTypedef SDCardInfo;
extern DMA_HandleTypeDef hdma_sdiorx;
extern DMA_HandleTypeDef hdma_sdiotx;

/* SDIO init function */
void MX_SDIO_SD_Init(void)
{

  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 120;
  HAL_SD_Init(&hsd, &SDCardInfo);

}

void HAL_SD_MspInit(SD_HandleTypeDef* hsd){
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hsd->Instance==SDIO)
  {
    /* Peripheral clock enable */
    __SDIO_CLK_ENABLE();
  
  /**SDIO GPIO Configuration  
  PC8   ------> SDIO_D0
  PC12   ------> SDIO_CK
  PD2   ------> SDIO_CMD 
  */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    /* Peripheral DMA init*/
  
    hdma_sdiorx.Instance = DMA2_Stream3;
    hdma_sdiorx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdiorx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sdiorx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdiorx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdiorx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdiorx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdiorx.Init.Mode = DMA_PFCTRL;
    hdma_sdiorx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_sdiorx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdiorx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdiorx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdiorx.Init.PeriphBurst = DMA_PBURST_INC4;
    HAL_DMA_Init(&hdma_sdiorx);

    hdma_sdiotx.Instance = DMA2_Stream6;
    hdma_sdiotx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdiotx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sdiotx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdiotx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdiotx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdiotx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdiotx.Init.Mode = DMA_PFCTRL;
    hdma_sdiotx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_sdiotx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdiotx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdiotx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdiotx.Init.PeriphBurst = DMA_PBURST_INC4;
    HAL_DMA_Init(&hdma_sdiotx);

    /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one stream to perform all the requested DMAs. */
    __HAL_LINKDMA(hsd,hdmarx,hdma_sdiorx);

    __HAL_LINKDMA(hsd,hdmatx,hdma_sdiotx);
    
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* hsd){
  if(hsd->Instance==SDIO)
  {
    /* Peripheral clock disable */
    __SDIO_CLK_DISABLE();
  
  /**SDIO GPIO Configuration  
  PC8   ------> SDIO_D0
  PC12   ------> SDIO_CK
  PD2   ------> SDIO_CMD 
  */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8|GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

  }
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
  htim2.Init.Prescaler = (96000 / SAMPLING_RATE) - 1;
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

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc){
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hadc->Instance==ADC1)
  {
    /* Peripheral clock enable */
    __ADC1_CLK_ENABLE();
 
  /**ADC1 GPIO Configuration  
  PA0/WKUP   ------> ADC1_IN0
  */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 
    /* Peripheral DMA init*/
 
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_adc1);
 
    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);
 
  }else if(hadc->Instance==ADC2){
    /* Peripheral clock enable */
    __ADC2_CLK_ENABLE();
 
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
 
    /* Peripheral DMA init*/
 
    hdma_adc2.Instance = DMA2_Stream2;
    hdma_adc2.Init.Channel = DMA_CHANNEL_1;
    hdma_adc2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_adc2.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_adc2.Init.Mode = DMA_CIRCULAR;
    hdma_adc2.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_adc2);
 
    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc2);
 
  }
}
 
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc){
  if(hadc->Instance==ADC1)
  {
    /* Peripheral clock disable */
    __ADC1_CLK_DISABLE();
 
  /**ADC1 GPIO Configuration  
  PA0/WKUP   ------> ADC1_IN0
  */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
 
    /* Peripheral DMA DeInit*/
     HAL_DMA_DeInit(hadc->DMA_Handle);
  }else if(hadc->Instance==ADC2)
  {
    /* Peripheral clock disable */
    __ADC2_CLK_DISABLE();
 
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2);
 
    /* Peripheral DMA DeInit*/
     HAL_DMA_DeInit(hadc->DMA_Handle);
  }
}
 
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac){
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hdac->Instance==DAC)
  {
    /* Peripheral clock enable */
    __DAC_CLK_ENABLE();
 
  /**DAC GPIO Configuration  
  PA5   ------> DAC_OUT2
  */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 
    /* Peripheral DMA init*/
 
    hdma_dac2.Instance = DMA1_Stream6;
    hdma_dac2.Init.Channel = DMA_CHANNEL_7;
    hdma_dac2.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_dac2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dac2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dac2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dac2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_dac2.Init.Mode = DMA_CIRCULAR;
    hdma_dac2.Init.Priority = DMA_PRIORITY_LOW;
    hdma_dac2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_dac2);
 
    __HAL_LINKDMA(hdac,DMA_Handle2,hdma_dac2);
 
  }
}
 
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac){
  if(hdac->Instance==DAC)
  {
    /* Peripheral clock disable */
    __DAC_CLK_DISABLE();
 
  /**DAC GPIO Configuration  
  PA5   ------> DAC_OUT2
  */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
 
    /* Peripheral DMA DeInit*/
     HAL_DMA_DeInit(hdac->DMA_Handle2);
  }
}
 
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base){
  if(htim_base->Instance==TIM2)
  {
    /* Peripheral clock enable */
    __TIM2_CLK_ENABLE();
  }
}
 
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base){
  if(htim_base->Instance==TIM2)
  {
    /* Peripheral clock disable */
    __TIM2_CLK_DISABLE();
 
  }
}
void HAL_LTDC_MspInit(LTDC_HandleTypeDef* hltdc){
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* hltdc){
}
