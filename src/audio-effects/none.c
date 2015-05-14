#include "none.h"
#include "FreeRTOS.h"

void None(q31_t* pData, void *opaque){
    return;
}

void delete_None(void *opaque){
    return;
}

void adjust_None(void *opaque, uint8_t* values){
    return;
}

void getParam_None(void *opaque, struct parameter_t* param[], uint8_t* paramNum){
    return;
}

struct Effect_t* new_None(){
    return NULL;
}

