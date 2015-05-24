#ifndef __VOLUME_H__
#define __VOLUME_H__

#include "base-effect.h"
#include "volume_img.bmp.h"

struct Volume_t{
    struct Effect_t parent;
    struct parameter_t gain;
    q31_t cache;
};

struct Effect_t* new_Volume();

static struct EffectType_t VolumeId __attribute__((unused)) = {
    .name = "Volume",
    .image = volume_img,
    .Init = new_Volume
};


#endif //__VOLUME_H__
