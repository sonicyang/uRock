/**
  ******************************************************************************
  * File Name          : stm32f4xx_hal_msp.c
  * Date               : 29/04/2015 21:52:51
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "setting.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/
/* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

extern DMA_HandleTypeDef hdma_sai1_a;
extern DMA_HandleTypeDef hdma_sai1_b;
int SAI1_client =0;

extern DMA_HandleTypeDef hdma_adc2;

extern DMA_HandleTypeDef hdma_sdiorx;
extern DMA_HandleTypeDef hdma_sdiotx;

void HAL_SAI_MspInit(SAI_HandleTypeDef* hsai)
{

  GPIO_InitTypeDef GPIO_InitStruct;
/* SAI1 */
    if(hsai->Instance==SAI1_Block_A)
    {
    /* Peripheral clock enable */
    if (SAI1_client == 0)
    {
      __SAI1_CLK_ENABLE();

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(SAI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SAI1_IRQn);
    }
    SAI1_client ++;
    
    /**SAI1_A_Block_A GPIO Configuration    
    PE2     ------> SAI1_MCLK_A
    PE4     ------> SAI1_FS_A
    PE5     ------> SAI1_SCK_A
    PE6     ------> SAI1_SD_A 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* Peripheral DMA init*/
    
    hdma_sai1_a.Instance = DMA2_Stream1;
    hdma_sai1_a.Init.Channel = DMA_CHANNEL_0;
    hdma_sai1_a.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sai1_a.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sai1_a.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sai1_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_sai1_a.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_sai1_a.Init.Mode = DMA_CIRCULAR;
    hdma_sai1_a.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_sai1_a.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_sai1_a);

    /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one stream to perform all the requested DMAs. */
    __HAL_LINKDMA(hsai,hdmarx,hdma_sai1_a);

    __HAL_LINKDMA(hsai,hdmatx,hdma_sai1_a);

    }
    if(hsai->Instance==SAI1_Block_B)
    {
      /* Peripheral clock enable */
      if (SAI1_client == 0)
      {
      __SAI1_CLK_ENABLE();

      /* Peripheral interrupt init*/
      HAL_NVIC_SetPriority(SAI1_IRQn, 5, 0);
      HAL_NVIC_EnableIRQ(SAI1_IRQn);
      }
    SAI1_client ++;
    
    /**SAI1_B_Block_B GPIO Configuration    
    PE3     ------> SAI1_SD_B 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* Peripheral DMA init*/
    
    hdma_sai1_b.Instance = DMA2_Stream4;
    hdma_sai1_b.Init.Channel = DMA_CHANNEL_1;
    hdma_sai1_b.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sai1_b.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sai1_b.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sai1_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sai1_b.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sai1_b.Init.Mode = DMA_CIRCULAR;
    hdma_sai1_b.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_sai1_b.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_sai1_b);

    /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one stream to perform all the requested DMAs. */
    __HAL_LINKDMA(hsai,hdmarx,hdma_sai1_b);
    __HAL_LINKDMA(hsai,hdmatx,hdma_sai1_b);
    }
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef* hsai)
{

/* SAI1 */
    if(hsai->Instance==SAI1_Block_A)
    {
    SAI1_client --;
    if (SAI1_client == 0)
      {
      /* Peripheral clock disable */ 
       __SAI1_CLK_DISABLE();
      HAL_NVIC_DisableIRQ(SAI1_IRQn);
      }
    
    /**SAI1_A_Block_A GPIO Configuration    
    PE2     ------> SAI1_MCLK_A
    PE4     ------> SAI1_FS_A
    PE5     ------> SAI1_SCK_A
    PE6     ------> SAI1_SD_A 
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6);

    HAL_DMA_DeInit(hsai->hdmarx);
    HAL_DMA_DeInit(hsai->hdmatx);
    }
    if(hsai->Instance==SAI1_Block_B)
    {
    SAI1_client --;
      if (SAI1_client == 0)
      {
      /* Peripheral clock disable */
      __SAI1_CLK_DISABLE();
      HAL_NVIC_DisableIRQ(SAI1_IRQn);
      }
    
    /**SAI1_B_Block_B GPIO Configuration    
    PE3     ------> SAI1_SD_B 
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3);

    HAL_DMA_DeInit(hsai->hdmarx);
    HAL_DMA_DeInit(hsai->hdmatx);
    }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc){
  uint32_t i;

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hadc->Instance==ADC2)
  {
    /* Peripheral clock enable */
    __ADC2_CLK_ENABLE();
 
    GPIO_InitStruct.Pin = 0;
    for(i = 0; i < POT_NUM; i++)
      GPIO_InitStruct.Pin |= POT_PINS[i];
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(POT_PORT, &GPIO_InitStruct);
 
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
  if(hadc->Instance==ADC2)
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
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* SDIO IRQ */
    HAL_NVIC_SetPriority(SDIO_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);
    
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

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
