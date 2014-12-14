#ifndef __LOWFREQOSC__
#define __LOWFREQOSC__

#include "stm32f4xx_hal.h"
#define ARM_MATH_CM4
#include "arm_math.h"
#include "core_cmInstr.h"

#include "setting.h"
#include "helper.h"

struct LFO_t{
    float (*next)(struct LFO_t* opaque);
    float lowerBound;
    float upperBound;
    uint32_t period;
    uint32_t ptr;
};

void new_LFO(struct LFO_t *opaque, float u, float l, uint32_t p);
void adjust_LFO_speed(struct LFO_t *opaque, uint32_t p);

#endif
