#ifndef __DISTORTION_H__
#define __DISTORTION_H__

#include "base-effect.h"

struct Distortion_t{
    struct Effect_t parent;
    struct parameter_t gain;
    struct parameter_t volume;
};

struct Effect_t* new_Distortion(struct Distortion_t* opaque);

#endif //__DISTORTION_H__
