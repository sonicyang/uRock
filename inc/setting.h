#ifndef __SETTING_H__
#define __SETTING_H__

#include "stm32f4xx_hal.h"

/* =====================================
 * Hardward wiring related configuration
 * ====================================*/
#define CONFIG_BUTTON_NUM   4
#define CONFIG_BUTTON_PORT  GPIOA
#define CONFIG_BUTTON1_PIN  GPIO_PIN_0
#define CONFIG_BUTTON2_PIN  GPIO_PIN_15
#define CONFIG_BUTTON3_PIN  GPIO_PIN_10
#define CONFIG_BUTTON4_PIN  GPIO_PIN_9
static const uint32_t CONFIG_BUTTON_PINS[4] = {CONFIG_BUTTON1_PIN, CONFIG_BUTTON2_PIN, CONFIG_BUTTON3_PIN, CONFIG_BUTTON4_PIN};

#define POT_NUM     4
#define POT_PORT    GPIOC
#define POT1_PIN    GPIO_PIN_1
#define POT2_PIN    GPIO_PIN_3
#define POT3_PIN    GPIO_PIN_4
#define POT4_PIN    GPIO_PIN_0
static const uint32_t POT_PINS[4] = {POT1_PIN, POT2_PIN, POT3_PIN, POT4_PIN};

#define PEDAL_PORT  GPIOA
#define PEDAL_PIN   GPIO_PIN_7


/* ===============================
 * Audio I/O related configuration
 * ==============================*/
/* Pipeline configuration */
#define BUFFER_NUM 2
#define SAMPLE_NUM 256

#define STAGE_NUM 8
#define STAGE_AVALIABLE 6
#define PIPE_LENGTH 2 * STAGE_NUM

/* Audio sample Parameter */
#define NORM_VALUE 0x65000000 

#define SAMPLING_RATE 96 /* In Kilo Hz, Make Sure it can divide 45000*/
#define SAMPLE_PERIOD (1.0f / SAMPLING_RATE)
#define SAMPLE_MAX 16384

#define DITHERING_AMP 0;

/* ============================
 * Effect related Configuration
 * ===========================*/
#define EFFECT_NUM 10
#define MAX_EFFECT_PARAM 4

#define MAX_CONFIG_NUM 4
#define Q_1 65536 // 2 ^ (32 - resolution) = 2 ^ 16
#define Q_MULT_SHIFT 15 // resolution - 1 = 15

#define BLOCK_PERIOD ((float)SAMPLE_NUM / SAMPLING_RATE)

/* ======================================
 * SDRAM Delay Line related Configuration
 * =====================================*/
#define DELAY_BANK(x) DELAY_BANK_##x

#define DELAY_BANK_0 0xD0200000
#define DELAY_BANK_1 0xD0300000
#define DELAY_BANK_2 0xD0400000
#define DELAY_BANK_3 0xD0500000
#define DELAY_BANK_4 0xD0600000

#endif //__SETTING_H__
