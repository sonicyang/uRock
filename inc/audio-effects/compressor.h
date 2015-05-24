#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include "base-effect.h"
#include "compressor_img.bmp.h"

struct Compressor_t{
    struct Effect_t parent;
    struct parameter_t attack;
    struct parameter_t threshold;
    struct parameter_t ratio;
    float env;
};

struct Effect_t* new_Compressor();

static struct EffectType_t CompressorId __attribute__((unused)) = {
    .name = "Compressor",
    .image = compressor_img,
    .Init = new_Compressor
};
#endif //__COMPRESSOR_H__
