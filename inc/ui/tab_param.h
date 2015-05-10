#ifndef __TAB_PARAM_H__
#define __TAB_PARAM_H__

#include "tab_base.h"

#include "setting.h"

#include "gfxconf.h"
#include "gfx.h"

struct tab_param_t{
    struct tab_t parent;

    GHandle label_effectTitle;
    GHandle label_param[MAX_EFFECT_PARAM];
    GHandle vbar_param[MAX_EFFECT_PARAM];

    GHandle btn_change;
    GHandle btn_back;
};

struct tab_t *tab_param_init(struct tab_param_t* opaque);

#endif //__TAB_PARAM_H__
