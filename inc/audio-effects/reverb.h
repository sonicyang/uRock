#ifndef __REVERB_H__
#define __REVERB_H__

#include "delay.h"

struct Reverb_t{
    struct Delay_t parent;
};

struct Effect_t* new_Reverb();

static struct EffectType_t ReverbId = {
    .name = "Reverb",
    .Init = new_Reverb
};

#endif //__REVERB_H__
