#include "uart.h"

UART_HandleTypeDef UartHandle;
__IO ITStatus UartReady = RESET;

HAL_StatusTypeDef UART_init(USART_TypeDef *uart, uint32_t BaudRate){
	UartHandle = (UART_HandleTypeDef) { .Instance = uart, .Init = { .BaudRate = BaudRate, .WordLength = UART_WORDLENGTH_8B, .StopBits = UART_STOPBITS_1,  .Parity = UART_PARITY_NONE, .HwFlowCtl = UART_HWCONTROL_NONE, .Mode = UART_MODE_TX_RX } };
	return HAL_UART_Init(&UartHandle);
}

HAL_StatusTypeDef UART_send(uint8_t* data, uint16_t length){	//blocking call
	HAL_StatusTypeDef status;
	UartReady = RESET;
	if((status = HAL_UART_Transmit_IT(&UartHandle, data, length)) != HAL_OK)
		return status;
	while(UartReady != SET);
	return HAL_OK;
}

HAL_StatusTypeDef UART_recv(uint8_t* buffer, uint16_t length){
	HAL_StatusTypeDef status;
	UartReady = RESET;
	if((status = HAL_UART_Receive_IT(&UartHandle, (uint8_t *)buffer, length)) != HAL_OK)
		return status;
	while(UartReady != SET);
	return HAL_OK;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle){
	/* Set transmission flag: trasfer complete*/
	UartReady = SET;
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle){
	/* Set transmission flag: trasfer complete*/
	UartReady = SET;
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart){  
	GPIO_InitTypeDef GPIO_InitStruct = {.Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST};
	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/*##-2- Configure peripheral GPIO ##########################################*/  
	/*##-3- Configure the NVIC for UART ########################################*/
	if(huart->Instance == USART1){		// tx/rx: PA9/PA10, PB6/PB7
		__USART1_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
	}else if(huart->Instance == USART2){	// tx/rx: PA2/PA3, PD5/PD6
		__USART2_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
	}else if(huart->Instance == USART3){	// tx/rx: PB10/PB11, PC10/PC11, PD8/PD9
		__USART3_CLK_ENABLE();
		__GPIOB_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART3_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART3_IRQn);
	}else if(huart->Instance == UART4){	// tx/rx: PA0/PA1, PC10/PC11
		__UART4_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
		GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(UART4_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(UART4_IRQn);
	}else if(huart->Instance == UART5){	// tx/rx: PC12/PD2
		__UART5_CLK_ENABLE();
		__GPIOC_CLK_ENABLE();
		__GPIOD_CLK_ENABLE();
		GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
		GPIO_InitStruct.Pin = GPIO_PIN_12;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = GPIO_PIN_2;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(UART5_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(UART5_IRQn);
	}else if(huart->Instance == USART6){	// tx/rx: PC6/PC7, PG14/PG9
		__USART6_CLK_ENABLE();
		__GPIOC_CLK_ENABLE();
		GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
		GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART6_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART6_IRQn);
	}else if(huart->Instance == UART7){	// tx/rx: PE8/PE7, PF7/PF6
		__UART7_CLK_ENABLE();
		__GPIOE_CLK_ENABLE();
		GPIO_InitStruct.Alternate = GPIO_AF8_UART7;
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_7;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(UART7_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(UART7_IRQn);
	}else if(huart->Instance == UART8){	// tx/rx: PE1/PE0
		__UART8_CLK_ENABLE();
		__GPIOE_CLK_ENABLE();
		GPIO_InitStruct.Alternate = GPIO_AF8_UART8;
		GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_0;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(UART8_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(UART8_IRQn);
	}else return;
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart){
	/*##-1- Reset peripherals ##################################################*/
	/*##-2- Disable peripherals and GPIO Clocks #################################*/
	/*##-3- Disable the NVIC for UART ##########################################*/
	if(huart->Instance == USART1){		// tx/rx: PA9/PA10, PB6/PB7
		__USART1_FORCE_RESET();
		__USART1_RELEASE_RESET();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
		HAL_NVIC_DisableIRQ(USART1_IRQn);
	}else if(huart->Instance == USART2){	// tx/rx: PA2/PA3, PD5/PD6
		__USART2_FORCE_RESET();
		__USART2_RELEASE_RESET();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
		HAL_NVIC_DisableIRQ(USART2_IRQn);
	}else if(huart->Instance == USART3){	// tx/rx: PB10/PB11, PC10/PC11, PD8/PD9
		__USART3_FORCE_RESET();
		__USART3_RELEASE_RESET();
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
		HAL_NVIC_DisableIRQ(USART3_IRQn);
	}else if(huart->Instance == UART4){	// tx/rx: PA0/PA1, PC10/PC11
		__UART4_FORCE_RESET();
		__UART4_RELEASE_RESET();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0 | GPIO_PIN_1);
		HAL_NVIC_DisableIRQ(UART4_IRQn);
	}else if(huart->Instance == UART5){	// tx/rx: PC12/PD2
		__UART5_FORCE_RESET();
		__UART5_RELEASE_RESET();
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);
		HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
		HAL_NVIC_DisableIRQ(UART5_IRQn);
	}else if(huart->Instance == USART6){	// tx/rx: PC6/PC7, PG14/PG9
		__USART6_FORCE_RESET();
		__USART6_RELEASE_RESET();
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6 | GPIO_PIN_7);
		HAL_NVIC_DisableIRQ(USART6_IRQn);
	}else if(huart->Instance == UART7){	// tx/rx: PE8/PE7, PF7/PF6
		__UART7_FORCE_RESET();
		__UART7_RELEASE_RESET();
		HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0 | GPIO_PIN_1);
		HAL_NVIC_DisableIRQ(UART7_IRQn);
	}else if(huart->Instance == UART8){	// tx/rx: PE1/PE0
		__UART8_FORCE_RESET();
		__UART8_RELEASE_RESET();
		HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0 | GPIO_PIN_1);
		HAL_NVIC_DisableIRQ(UART8_IRQn);
	}else return;
}

/**
  * @brief  These function handles UART interrupt request.  
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream 
  *         used for USART data transmission     
  */
void USART1_IRQHandler(void){
  HAL_UART_IRQHandler(&UartHandle);
}
void USART2_IRQHandler(void){
  HAL_UART_IRQHandler(&UartHandle);
}
void USART3_IRQHandler(void){
  HAL_UART_IRQHandler(&UartHandle);
}
void UART4_IRQHandler(void){
  HAL_UART_IRQHandler(&UartHandle);
}
void UART5_IRQHandler(void){
  HAL_UART_IRQHandler(&UartHandle);
}
void USART6_IRQHandler(void){
  HAL_UART_IRQHandler(&UartHandle);
}
void UART7_IRQHandler(void){
  HAL_UART_IRQHandler(&UartHandle);
}
void UART8_IRQHandler(void){
  HAL_UART_IRQHandler(&UartHandle);
}
