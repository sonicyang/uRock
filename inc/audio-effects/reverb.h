#ifndef __REVERB_H__
#define __REVERB_H__

#include "delay.h"

struct Reverb_t{
    struct Delay_t parent;
};

struct Effect_t* new_Reverb(struct Reverb_t* opaque);

#endif //__REVERB_H__
