#ifndef __LOWFREQOSC__
#define __LOWFREQOSC__

#include "stm32f4xx_hal.h"
#include "arm_math.h"

#include "setting.h"
#include "helper.h"

struct LFO_t{
    float (*next)(struct LFO_t* opaque);
    float lowerBound;
    float upperBound;
    uint32_t feq;
    uint32_t ptr;
    float cache0;
    float cache1;
};

void new_LFO(struct LFO_t *opaque, float u, float l, uint32_t p);
void adjust_LFO_speed(struct LFO_t *opaque, uint32_t p);

#endif
