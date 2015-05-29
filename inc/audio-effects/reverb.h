#ifndef __REVERB_H__
#define __REVERB_H__

#include "delay.h"
#include "reverb_img.bmp.h"

struct Reverb_t{
    struct Delay_t parent;
};

struct Effect_t* new_Reverb();

static struct EffectType_t ReverbId __attribute__((unused)) = {
    .name = "Reverb",
    .image = reverb_img,
    .Init = new_Reverb
}; 
#endif //__REVERB_H__
