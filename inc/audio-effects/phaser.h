#ifndef __PHASER_H__
#define __PHASER_H__

#include "base-effect.h"
#include "lowFreqOsc.h"

struct Phaser_t{
    struct Effect_t parent;
    struct LFO_t lfo;
    struct parameter_t speed;
};

struct Effect_t* new_Phaser();

#endif //__PHASER_H__
