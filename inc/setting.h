#ifndef __SETTING_H__
#define __SETTING_H__

#define DITHERING_AMP 0;

#define MAX_EFFECT_PARAM 4

#define BUFFER_NUM 2
#define STAGE_NUM 8
#define SAMPLE_NUM 256

#define SAMPLE_MAX 2047

#define MAX_CONFIG_NUM 4

#define SAMPLING_RATE 44.1 /* In Kilo Hz, Make Sure it can divide 45000*/
#define SAMPLE_PERIOD (1.0f / SAMPLING_RATE)

#define Q_1 1048576
#define Q_MULT_SHIFT 11

#define BLOCK_PERIOD ((float)SAMPLE_NUM / SAMPLING_RATE)

#define DELAY_BANK(x) DELAY_BANK_##x

#define DELAY_BANK_0 0xD0200000
#define DELAY_BANK_1 0xD0300000
#define DELAY_BANK_2 0xD0400000
#define DELAY_BANK_3 0xD0500000
#define DELAY_BANK_4 0xD0600000

#endif //__SETTING_H__
