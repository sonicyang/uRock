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
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#include "cmsis_os.h"

#include "MspInit.h"

//static void Error_Handler(void);
static void SystemClock_Config(void);

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

osThreadId LEDThreadId;
static void LEDThread(void const *argument);

osThreadId RenderingThreadId;
static void RenderingThread(void const *argument);

int
main(void)
{
	/* Configure the system clock to 180 Mhz */
	SystemClock_Config();

	/* STM32F4xx HAL library initialization:
	   - Configure the Flash prefetch, instruction and Data caches
	   - Configure the Systick to generate an interrupt each 1 msec
	   - Set NVIC Group Priority to 4
	   - Global MSP (MCU Support Package) initialization
	   */
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

	osThreadDef(LED, LEDThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	LEDThreadId = osThreadCreate (osThread(LED), NULL);

	osThreadDef(RENDERING, RenderingThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	RenderingThreadId = osThreadCreate (osThread(RENDERING), NULL);

	osKernelStart (NULL, NULL);

	/* We should never get here as control is now taken by the scheduler */
	while (1)
		;
}

static void LEDThread(void const *argument){
	for(;;){
		osDelay(300);
		BSP_LED_Toggle(LED3);
	}
}

static void RenderingThread(void const *argument){
	TS_StateTypeDef tp;
	uint16_t x = 50;
	uint16_t y = 50;
	int8_t vx = 1;
	int8_t vy = 1;

	BSP_LCD_SetFont(Font8x8);

	while (1) {
		BSP_TS_GetState(&tp);
		if (tp.TouchDetected == 1) {
			BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
		} else {
			BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
		}

		x += vx;
		y += vy;

		if (x == 0 || x + 30 == BSP_LCD_GetXSize() - 1)
			vx *= -1;
		if (y == 0 || y + 30 == BSP_LCD_GetYSize() - 1)
			vy *= -1;

		BSP_LCD_Clear(LCD_COLOR_WHITE);
		BSP_LCD_DisplayStringAt(0, 0, (uint8_t*) "123456789", CENTER_MODE);
		BSP_LCD_FillRect(x, y, 30, 30);
		osDelay(10);
	}
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
