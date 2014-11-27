#include <stdint.h>
#include "hash-djb2.h"
#include "osdebug.h"

uint32_t hash_djb2(const uint8_t * str, ssize_t _max) {
    uint32_t hash = 5381;
    uint32_t max = (uint32_t) _max;
    int c;
    
    while (((c = *str++)) && max--) {
        hash = ((hash << 5) + hash) ^ c;
    }
    
    return hash;
}
