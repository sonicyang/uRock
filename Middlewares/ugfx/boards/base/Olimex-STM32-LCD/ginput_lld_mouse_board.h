/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

#define ADC_NUM_CHANNELS   2
#define ADC_BUF_DEPTH      1

static const ADCConversionGroup adc_y_config = { 
    FALSE,
    ADC_NUM_CHANNELS,
    0,
    0,
    0, 0,    
    0, 0,    
    ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
    0,    
    ADC_SQR3_SQ2_N(ADC_CHANNEL_IN12) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN13)
};

static const ADCConversionGroup adc_x_config = { 
    FALSE,
    ADC_NUM_CHANNELS,
    0,
    0,
    0, 0,
    0, 0,
    ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
    0,  
    ADC_SQR3_SQ2_N(ADC_CHANNEL_IN10) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11)
};

static inline void init_board(void) {
	adcStart(&ADCD1, 0);
}

static inline void aquire_bus(void) {

}

static inline void release_bus(void) {

}

static inline void setup_x(void) {
	palSetPadMode(GPIOC, 0, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 2, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOC, 3, PAL_MODE_OUTPUT_PUSHPULL);

	palSetPad(GPIOC, 2);
	palClearPad(GPIOC, 3); 
	gfxSleepMilliseconds(1);
}

static inline void setup_y(void) {
	palSetPadMode(GPIOC, 2, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 3, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 0, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOC, 1, PAL_MODE_OUTPUT_PUSHPULL);

	palSetPad(GPIOC, 1);
	palClearPad(GPIOC, 0);
	gfxSleepMilliseconds(1);
}

static inline void setup_z(void) {
	palSetPadMode(GPIOC, 0, PAL_MODE_INPUT_PULLDOWN);
	palSetPadMode(GPIOC, 1, PAL_MODE_INPUT);
	palSetPadMode(GPIOC, 2, PAL_MODE_INPUT);
	palSetPadMode(GPIOC, 3, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPad(GPIOC, 3);
}

static inline uint16_t read_x(void) {
	uint16_t val1, val2;
    adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

	palSetPad(GPIOC, 2);
    palClearPad(GPIOC, 3); 
    gfxSleepMilliseconds(1);
    adcConvert(&ADCD1, &adc_x_config, samples, ADC_BUF_DEPTH);  
    val1 = ((samples[0] + samples[1])/2);
    
	palClearPad(GPIOC, 2);
    palSetPad(GPIOC, 3);
    gfxSleepMilliseconds(1);
    adcConvert(&ADCD1, &adc_x_config, samples, ADC_BUF_DEPTH);
    val2 = ((samples[0] + samples[1])/2);
    
	return ((val1+((1<<12)-val2))/4);
}

static inline uint16_t read_y(void) {
	uint16_t val1, val2;
    adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

    palSetPad(GPIOC, 1);
    palClearPad(GPIOC, 0);
    gfxSleepMilliseconds(1);
    adcConvert(&ADCD1, &adc_y_config, samples, ADC_BUF_DEPTH);
    val1 = ((samples[0] + samples[1])/2);

    palClearPad(GPIOC, 1);
    palSetPad(GPIOC, 0);
    gfxSleepMilliseconds(1);
    adcConvert(&ADCD1, &adc_y_config, samples, ADC_BUF_DEPTH);
    val2 = ((samples[0] + samples[1])/2);
    
	return ((val1+((1<<12)-val2))/4);
}

static inline uint16_t read_z(void) {
	if (palReadPad(GPIOC, 0))
		return 100;
	else
		return 0;
}

#endif /* _GINPUT_LLD_MOUSE_BOARD_H */

