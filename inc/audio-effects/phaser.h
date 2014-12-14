#ifndef __PHASER_H__
#define __PHASER_H__

#include "base-effect.h"

struct Phaser_t{
    struct Effect_t parent;
};

struct Effect_t* new_Phaser(struct Phaser_t* opaque);

#endif //__PHASER_H__
