#ifndef __OVERDRIVE_H__
#define __OVERDRIVE_H__

#include "base-effect.h"

struct Overdrive_t{
    struct Effect_t parent;
    struct parameter_t gain;
    struct parameter_t volume;
    struct parameter_t ratio;
};

struct Effect_t* new_Overdrive(struct Overdrive_t* opaque);

#endif //__OVERDRIVE_H__
