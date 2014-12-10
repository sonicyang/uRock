#ifndef __VOLUME_H__
#define __VOLUME_H__

#include "base-effect.h"

struct Volume_t{
    struct Effect_t parent;
    struct parameter_t gain;
};

struct Effect_t* new_Volume(struct Volume_t* opaque);

#endif //__VOLUME_H__
