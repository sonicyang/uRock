#ifndef __MSPINIT_H__
#define __MSPINIT_H__

#include "stm32f4xx_hal.h"

void MX_ADC1_Init(void);
void MX_DAC_Init(void);
void MX_TIM2_Init(void);
void MX_DMA_Init(void);
void MX_GPIO_Init(void);

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc);
 
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc);
 
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac);
 
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac);

#endif //__MSPINIT_H__
