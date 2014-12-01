#include "MspInit.h"

extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_dac2;

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc){
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hadc->Instance==ADC1)
  {
    /* Peripheral clock enable */
    __ADC1_CLK_ENABLE();
 
  /**ADC1 GPIO Configuration  
  PA0/WKUP   ------> ADC1_IN0
  */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 
    /* Peripheral DMA init*/
 
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_adc1);
 
    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);
 
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
    hdma_dac2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_dac2.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
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

