#include "reverb.h"
#include "helper.h"

struct Effect_t* new_Reverb(){
    struct Delay_t *tmp = (struct Delay_t*)new_Delay();

    strcpy(tmp->parent.name, "Reverb");
    tmp->parent.FXid = &ReverbId;

    tmp->attenuation.upperBound = -5.0f;
    tmp->attenuation.lowerBound = -30.0f;
    tmp->attenuation.value = -10.0f;

    tmp->delayTime.upperBound = 50.0f;
    tmp->delayTime.lowerBound = 10.0f;
    tmp->delayTime.value = 35.0f;

    return (struct Effect_t*)tmp;
}


