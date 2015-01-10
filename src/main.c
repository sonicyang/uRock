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

#include "MspInit.h"
#include "ui.h"
#include "spu.h"

#include "sys_defs.h"

//static void Error_Handler(void);
static void SystemClock_Config(void);

/* Private variables ---------------------------------------------------------*/
SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypedef SDCardInfo;
DMA_HandleTypeDef hdma_sdiorx;
DMA_HandleTypeDef hdma_sdiotx;

uint8_t SD_DriverNum;      /* FatFS SD part */
char SD_Path[4];           /* SD card logical drive path */
FATFS FatFs;
FIL fil;

#define COLOR_SIZE	20
#define PEN_SIZE	20
#define POFFSET		3

#define COLOR_BOX(a)		(ev.x >= a && ev.x <= a + COLOR_SIZE)
#define PEN_BOX(a)			(ev.y >= a && ev.y <= a + COLOR_SIZE)
#define GET_COLOR(a)		(COLOR_BOX(a * COLOR_SIZE + POFFSET))
#define GET_PEN(a)			(PEN_BOX(a * 2 * PEN_SIZE + POFFSET))
#define DRAW_COLOR(a)		(a * COLOR_SIZE + POFFSET)
#define DRAW_PEN(a)			(a * 2 * PEN_SIZE + POFFSET)
#define DRAW_AREA(x, y)		(x >= PEN_SIZE + POFFSET + 3 && x <= gdispGetWidth() && \
							 y >= COLOR_SIZE + POFFSET + 3 && y <= gdispGetHeight())

/* Task priorities. */
#define mainFLASH_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainLCD_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )

void drawScreen(void)
{
	char *msg = "uGFX";
	font_t		font1, font2;

	font1 = gdispOpenFont("DejaVuSans24*");
	font2 = gdispOpenFont("DejaVuSans12*");

	gdispClear(White);
	gdispDrawString(gdispGetWidth()-gdispGetStringWidth(msg, font1)-3, 3, msg, font1, Black);

	/* colors */
	gdispFillArea(0 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Black);	/* Black */
	gdispFillArea(1 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, uRed);		/* uRed */
	gdispFillArea(2 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Yellow);	/* Yellow */
	gdispFillArea(3 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, uGreen);	/* uGreen */
	gdispFillArea(4 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, uBlue);		/* uBlue */
	gdispDrawBox (5 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Black);	/* White */

	/* pens */
	gdispFillStringBox(POFFSET * 2, DRAW_PEN(1), PEN_SIZE, PEN_SIZE, "1", font2, White, Black, justifyCenter);
	gdispFillStringBox(POFFSET * 2, DRAW_PEN(2), PEN_SIZE, PEN_SIZE, "2", font2, White, Black, justifyCenter);
	gdispFillStringBox(POFFSET * 2, DRAW_PEN(3), PEN_SIZE, PEN_SIZE, "3", font2, White, Black, justifyCenter);
	gdispFillStringBox(POFFSET * 2, DRAW_PEN(4), PEN_SIZE, PEN_SIZE, "4", font2, White, Black, justifyCenter);
	gdispFillStringBox(POFFSET * 2, DRAW_PEN(5), PEN_SIZE, PEN_SIZE, "5", font2, White, Black, justifyCenter);

	gdispCloseFont(font1);
	gdispCloseFont(font2);
}

GEventMouse		ev;

/* GFX notepad demo */
static void prvLCDTask(void *pvParameters)
{
	color_t color = Black;
	uint16_t pen = 0;

	( void ) pvParameters;

	gfxInit();
	ginputGetMouse(0);

	drawScreen();

	while (TRUE) {
		ginputGetMouseStatus(0, &ev);
		if (!(ev.current_buttons & GINPUT_MOUSE_BTN_LEFT))
			continue;

		/* inside color box ? */
		if(ev.y >= POFFSET && ev.y <= COLOR_SIZE) {
			     if(GET_COLOR(0)) 	color = Black;
			else if(GET_COLOR(1))	color = uRed;
			else if(GET_COLOR(2))	color = Yellow;
			else if(GET_COLOR(3))	color = uGreen;
			else if(GET_COLOR(4))	color = uBlue;
			else if(GET_COLOR(5))	color = White;

		/* inside pen box ? */
		} else if(ev.x >= POFFSET && ev.x <= PEN_SIZE) {
			     if(GET_PEN(1))		pen = 0;
			else if(GET_PEN(2))		pen = 1;
			else if(GET_PEN(3))		pen = 2;
			else if(GET_PEN(4))		pen = 3;
			else if(GET_PEN(5))		pen = 4;

		/* inside drawing area ? */
		} else if(DRAW_AREA(ev.x, ev.y)) {
			if(pen == 0)
				gdispDrawPixel(ev.x, ev.y, color);
			else
				gdispFillCircle(ev.x, ev.y, pen, color);
		}
	}
}

int main(void)
{
	/* Configure the hardware ready to run the test. */
	SystemClock_Config();

	HAL_Init();

	/* Start the LCD task */
	xTaskCreate( prvLCDTask, "LCD", 256, NULL, mainLCD_TASK_PRIORITY, NULL );

	/* Start the scheduler. */
	vTaskStartScheduler();
}


void vApplicationTickHook(void)
{
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
