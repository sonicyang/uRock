#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef UART_init(USART_TypeDef *uart, uint32_t BaudRate);
HAL_StatusTypeDef UART_send(uint8_t* data, uint16_t length);
HAL_StatusTypeDef UART_recv(uint8_t* buffer, uint16_t length);
void USARTx_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H */
