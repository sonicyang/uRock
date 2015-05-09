#ifndef __TAB_LIST_H__
#define __TAB_LIST_H__

#include "tab_base.h"

#include "setting.h"

#include "gfxconf.h"
#include "gfx.h"

struct tab_list_t{
    struct tab_t parent;

    GHandle label_uRock;
    GHandle btn_effectIndicate[STAGE_NUM];

    GHandle label_effectName[STAGE_NUM];
    GHandle btn_effectSwitch[STAGE_NUM];
    GHandle btn_playWav;
    GHandle btn_recordWav;
};

struct tab_t *tab_list_init(struct tab_list_t* opaque);

#endif //__TAB_LIST_H__
