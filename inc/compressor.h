#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include "base-effect.h"

struct Compressor_t{
    struct Effect_t parent;
    struct parameter_t attack;
    struct parameter_t threshold;
    struct parameter_t ratio;
    uint8_t pre_status;
    uint32_t count;
    float pre_r;
};

struct Effect_t* new_Compressor(struct Compressor_t* opaque);

#endif //__COMPRESSOR_H__
