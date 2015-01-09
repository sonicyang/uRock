#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"

extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern DMA_HandleTypeDef hdma_dac2;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac;

extern SD_HandleTypeDef hsd;
extern DMA_HandleTypeDef hdma_sdiorx;
extern DMA_HandleTypeDef hdma_sdiotx;

void NVIC_Init(void){

    /* SDIO IRQ */
    HAL_NVIC_SetPriority(SDIO_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);

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

void SDIO_IRQHandler(void){
    HAL_SD_IRQHandler(&hsd);
    return;
}

void DMA2_Stream3_IRQHandler(void){
    HAL_DMA_IRQHandler(&hdma_sdiorx);
    return;
}

void DMA2_Stream6_IRQHandler(void){
    HAL_DMA_IRQHandler(&hdma_sdiotx);
    return;
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
