#ifndef __HASH_DJB2_H__
#define __HASH_DJB2_H__

#include <unistd.h>

uint32_t hash_djb2(const uint8_t * str, ssize_t max);

#endif
