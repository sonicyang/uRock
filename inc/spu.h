#ifndef __SPU_H__
#define __SPU_H__

#include "setting.h"

#include "base-effect.h"

#include "none.h"

#include "volume.h"
#include "compressor.h"

#include "distortion.h"
#include "overdrive.h"

#include "equalizer.h"

#include "delay.h"
#include "reverb.h"

#include "phaser.h"
#include "flanger.h"

#include "peakFilter.h"
#include "shelvingFilter.h"
static struct EffectType_t* EFFECTS[EFFECT_NUM] __attribute__((unused)) = {
    &NoneId,
    &VolumeId,
    &CompressorId,
    &DistortionId,
    &OverdriveId,
    &DelayId,
    &ReverbId,
    &FlangerId,
    &EqualizerId,
    &PeakFilterId,
    &ShelvingFilterId,
};

void SignalProcessingUnit(void const * argument);

void attachEffect(uint32_t stage, uint32_t effectType);
const char *cvtToEffectName(uint32_t ee);
uint32_t cvtToEffectId(const char* name);
const struct Effect_t* const retriveStagedEffect(uint32_t stage);

#endif //__SPU_H__
