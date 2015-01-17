#include "tab_list.h"

#include "base-effect.h"

#include "gfxconf.h"
#include "gfx.h"
#include "src/gwin/sys_defs.h"
#include "cfgFunc.h"
#include "wavplayer.h"
#include "wavrecoder.h"

#include "setting.h"

#include "ui.h"

extern struct Effect_t *EffectList[STAGE_NUM];
extern int8_t controllingStage;

void tab_list_refresh(void* opaque){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;

    for(i = 0; i < STAGE_NUM; i++){
        if (EffectList[i]){
            gwinSetText(tmp->label_effectName[i], EffectList[i]->name, 0);

        }else{
            gwinSetText(tmp->label_effectName[i], "", 0);
        }
        if(i == controllingStage){
            gwinSetText(tmp->btn_effectIndicate[i], "->", 0);
        }else{
            gwinSetText(tmp->btn_effectIndicate[i], "", 0);
        }
    }

    return;
}

void tab_list_eHandle(void* opaque, GEventGWinButton* event){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;

    for(i = 0; i < STAGE_NUM; i++){
        if ((event)->button == tmp->btn_effectIndicate[i]){
            controllingStage = i;
        }
    }

    for(i = 0; i < STAGE_NUM; i++){
        if ((event)->button == tmp->btn_effectSwitch[i]){
            controllingStage = i;
            SwitchTab(SELECT_EFFECT_TAB);
        }
    }

    for (i = 0; i < 3; ++i){
        if ((event)->button == tmp->btn_configSwitch[i]){
            SaveStageSetting(currentConfig);
            currentConfig = i;
            ReadStageSetting(currentConfig);
        }
    }

    if ((event)->button == tmp->btn_playWav){
        if (EffectList[3])
            EffectList[3]->del(EffectList[3]);

        EffectList[3] = new_WavPlayer();
    }

    if ((event)->button == tmp->btn_recordWav){
        if (EffectList[3])
            EffectList[3]->del(EffectList[3]);

        EffectList[3] = new_WavRecoder();
    }

    return;
}

void tab_list_show(void* opaque){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;

    gwinSetVisible(tmp->label_uRock, TRUE);
    for(i = 0; i < STAGE_NUM; i++){
        gwinSetVisible(tmp->btn_effectIndicate[i], TRUE);
        gwinSetVisible(tmp->label_effectName[i], TRUE);
        gwinSetVisible(tmp->btn_effectSwitch[i], TRUE);
    }

    for (i = 0; i < 3; ++i)
        gwinSetVisible(tmp->btn_configSwitch[i], TRUE);

    gwinSetVisible(tmp->btn_playWav, TRUE);
    gwinSetVisible(tmp->btn_recordWav, TRUE);

    return;
}

void tab_list_hide(void* opaque){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;

    gwinSetVisible(tmp->label_uRock, FALSE);
    for(i = 0; i < STAGE_NUM; i++){
        gwinSetVisible(tmp->btn_effectIndicate[i], FALSE);
        gwinSetVisible(tmp->label_effectName[i], FALSE);
        gwinSetVisible(tmp->btn_effectSwitch[i], FALSE);
    }

    for (i = 0; i < 3; ++i)
        gwinSetVisible(tmp->btn_configSwitch[i], FALSE);

    gwinSetVisible(tmp->btn_playWav, FALSE);
    gwinSetVisible(tmp->btn_recordWav, FALSE);

    return;
}

struct tab_t *tab_list_init(struct tab_list_t* opaque){
    uint32_t i;
    GWidgetInit wi;

    /* StageTab */
    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    wi.g.x = 0;
    wi.g.y = 0;
    wi.g.width = 100;
    wi.g.height = 20;
    wi.text = "uRock";
    opaque->label_uRock = gwinLabelCreate(NULL, &wi);

    for(i = 0; i < STAGE_NUM; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 10;
        wi.g.y = 50 + 40 * i;
        wi.g.width = 30;
        wi.g.height = 30;
        wi.text = "";
        opaque->btn_effectIndicate[i] = gwinButtonCreate(NULL, &wi);

        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 50;
        wi.g.y = 50 + 40 * i;
        wi.g.width = 140;
        wi.g.height = 30;
        wi.text = "";
        opaque->label_effectName[i] = gwinLabelCreate(NULL, &wi);

        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 200;
        wi.g.y = 50 + 40 * i;
        wi.g.width = 30;
        wi.g.height = 30;
        wi.text = "";
        opaque->btn_effectSwitch[i] = gwinButtonCreate(NULL, &wi);
    }

    for (i = 0; i < 3; ++i){
        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 10 + 40 * i;
        wi.g.y = 250;
        wi.g.width = 30;
        wi.g.height = 30;
        wi.text = "";
        opaque->btn_configSwitch[i] = gwinButtonCreate(NULL, &wi);
    }

    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    wi.g.x = 200;
    wi.g.y = 250;
    wi.g.width = 30;
    wi.g.height = 30;
    wi.text = "P";
    opaque->btn_playWav = gwinButtonCreate(NULL, &wi);

    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    wi.g.x = 240;
    wi.g.y = 250;
    wi.g.width = 30;
    wi.g.height = 30;
    wi.text = "R";
    opaque->btn_recordWav = gwinButtonCreate(NULL, &wi);

    opaque->parent.show = tab_list_show;
    opaque->parent.hide = tab_list_hide;
    opaque->parent.refresh = tab_list_refresh;
    opaque->parent.eHandle = tab_list_eHandle;

    return (struct tab_t*)opaque;
}
