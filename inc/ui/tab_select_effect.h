#ifndef __TAB_SELECT_EFFECT_H__
#define __TAB_SELECT_EFFECT_H__

#include "tab_base.h"

#include "spu.h"

#include "setting.h"

#include "gfxconf.h"
#include "gfx.h"

struct tab_select_effect_t{
    struct tab_t parent;

    GHandle btn_effectTypes[EFFECT_NUM + 1];
    GHandle btn_back;
};

struct tab_t *tab_select_effect_init(struct tab_select_effect_t* opaque);

#endif //__TAB_SELECT_EFFECT_H__
