#include "tab_list.h"

#include "base-effect.h"
#include "spu.h"

#include "gfxconf.h"
#include "gfx.h"
#include "cfgFunc.h"
//#include "wavplayer.h"
//#include "wavrecoder.h"

#include "setting.h"

#include "ui.h"

#include "helper.h"

extern uint32_t selectedEffectStage;

void tab_list_refresh(void* opaque){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;
    char buf[32];

    for(i = 0; i < STAGE_NUM; i++){
        itoa(i + 1, buf);

        if(retriveStagedEffect(i)){
            strcat(buf, " ");
            strcat(buf, retriveStagedEffect(i)->name);
        }else{
            strcat(buf, " Empty");
        }

        gwinSetText(tmp->btn_effectIndicate[i], buf, 1);
    }

    return;
}

void tab_list_bHandle(void* opaque, GEventGWinButton* event){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;
    
    for(i = 0; i < STAGE_NUM; i++){
        if ((event)->gwin == tmp->btn_effectIndicate[i]){
            selectedEffectStage = i;
            SwitchTab(PARAM_TAB);
        }
    }

    for(i = 0; i < STAGE_NUM; i++){
        if ((event)->gwin == tmp->btn_effectSwitch[i]){
            selectedEffectStage = i;
            SwitchTab(PARAM_TAB);
        }
    }
    /*
    if ((event)->gwin == tmp->btn_playWav){
        if (EffectList[3])
            EffectList[3]->del(EffectList[3]);

        vTaskDelay(1500 / portTICK_RATE_MS);
        //EffectList[3] = new_WavPlayer();
    }

    if ((event)->gwin == tmp->btn_recordWav){
        if (EffectList[3])
            EffectList[3]->del(EffectList[3]);

        vTaskDelay(1500 / portTICK_RATE_MS);
        //EffectList[3] = new_WavRecoder();
    }
    */
    return;
}

void tab_list_show(void* opaque){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;

    gwinSetVisible(tmp->label_uRock, TRUE);
    for(i = 0; i < STAGE_NUM; i++){
        gwinSetVisible(tmp->btn_effectIndicate[i], TRUE);
        //gwinSetVisible(tmp->label_effectName[i], TRUE);
        //gwinSetVisible(tmp->btn_effectSwitch[i], TRUE);
    }

    //gwinSetVisible(tmp->btn_playWav, TRUE);
    //gwinSetVisible(tmp->btn_recordWav, TRUE);

    return;
}

void tab_list_hide(void* opaque){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;

    gwinSetVisible(tmp->label_uRock, FALSE);
    for(i = 0; i < STAGE_NUM; i++){
        gwinSetVisible(tmp->btn_effectIndicate[i], FALSE);
        //gwinSetVisible(tmp->label_effectName[i], FALSE);
        //gwinSetVisible(tmp->btn_effectSwitch[i], FALSE);
    }

    //gwinSetVisible(tmp->btn_playWav, FALSE);
    //gwinSetVisible(tmp->btn_recordWav, FALSE);

    return;
}

struct tab_t *tab_list_init(struct tab_list_t* opaque){
    uint32_t i;
    GWidgetInit wi;
    char buf[3];

    /* StageTab */
    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    wi.g.x = 30;
    wi.g.y = 0;
    wi.g.width = 240;
    wi.g.height = 20;
    wi.text = "uRock : YouRock";
    opaque->label_uRock = gwinLabelCreate(NULL, &wi);

    for(i = 0; i < STAGE_NUM / 2; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 10;
        wi.g.y = 50 + 60 * i;
        wi.g.width = 100;
        wi.g.height = 50;
        wi.text = "";
        opaque->btn_effectIndicate[i] = gwinButtonCreate(NULL, &wi);
    }
    for(; i < STAGE_NUM; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 130;
        wi.g.y = 50 + 60 * (7 - i);
        wi.g.width = 100;
        wi.g.height = 50;
        wi.text = "";
        opaque->btn_effectIndicate[i] = gwinButtonCreate(NULL, &wi);

        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 130;
        wi.g.y = 90 + 60 * (7 - i);
        wi.g.width = 10;
        wi.g.height = 10;
        itoa(i, buf);
        wi.text = buf;
        opaque->label_effectName[i] = gwinLabelCreate(NULL, &wi);
    }
/*
    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    wi.g.x = 10;
    wi.g.y = 210;
    wi.g.width = 100;
    wi.g.height = 100;
    wi.text = "P";
    opaque->btn_playWav = gwinButtonCreate(NULL, &wi);

    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    wi.g.x = 120;
    wi.g.y = 210;
    wi.g.width = 100;
    wi.g.height = 100;
    wi.text = "R";
    opaque->btn_recordWav = gwinButtonCreate(NULL, &wi);
    */
    opaque->parent.show = tab_list_show;
    opaque->parent.hide = tab_list_hide;
    opaque->parent.refresh = tab_list_refresh;
    opaque->parent.bHandle = tab_list_bHandle;
    

    return (struct tab_t*)opaque;
}
