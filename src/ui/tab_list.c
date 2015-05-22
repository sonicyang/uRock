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

#include "google.h"

extern uint32_t selectedEffectStage;
gdispImage ghImages[STAGE_AVALIABLE];

void tab_list_refresh(void* opaque){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;

    for(i = 0; i < STAGE_AVALIABLE; i++){

        gdispImageClose(ghImages + i);

        if(retriveStagedEffect(i)){
            gdispImageOpenMemory(ghImages + i, retriveStagedEffect(i)->FXid->image);
            gwinSetCustomDraw(tmp->btn_effectIndicate[i], gwinButtonDraw_Image, ghImages + i);
            gwinuRedraw(tmp->btn_effectIndicate[i]);
            gwinSetText(tmp->btn_effectIndicate[i], "", 0);
        }else{
            gdispImageOpenMemory(ghImages + i, EFFECTS[0]->image);
            gwinSetCustomDraw(tmp->btn_effectIndicate[i], gwinButtonDraw_Image, ghImages + i);
            gwinuRedraw(tmp->btn_effectIndicate[i]);
            gwinSetText(tmp->btn_effectIndicate[i], "Empty", 0);
        }

    }

    return;
}

void tab_list_bHandle(void* opaque, GEventGWinButton* event){
    struct tab_list_t *tmp = (struct tab_list_t*)opaque;
    uint32_t i;
    
    for(i = 0; i < STAGE_AVALIABLE; i++){
        if ((event)->gwin == tmp->btn_effectIndicate[i]){
            selectedEffectStage = i;
            SwitchTab(PARAM_TAB);
        }
    }

    for(i = 0; i < STAGE_AVALIABLE; i++){
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
    for(i = 0; i < STAGE_AVALIABLE; i++){
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
    for(i = 0; i < STAGE_AVALIABLE; i++){
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

    /* StageTab */
    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    wi.g.x = 30;
    wi.g.y = 0;
    wi.g.width = 240;
    wi.g.height = 20;
    wi.text = "uRock : YouRock";
    opaque->label_uRock = gwinLabelCreate(NULL, &wi);

    for(i = 0; i < STAGE_AVALIABLE / 2; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 50;
        wi.g.y = 50 + 70 * i;
        wi.g.width = 48;
        wi.g.height = 60;
        wi.text = "";
        opaque->btn_effectIndicate[i * 2] = gwinButtonCreate(NULL, &wi);
        gdispImageOpenMemory(ghImages + i * 2, EFFECTS[0]->image);
        gwinSetCustomDraw(opaque->btn_effectIndicate[i * 2], gwinButtonDraw_Image, ghImages + i * 2);

        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 140;
        wi.g.y = 50 + 70 * i;
        wi.g.width = 48;
        wi.g.height = 60;
        wi.text = "";
        opaque->btn_effectIndicate[i * 2 + 1] = gwinButtonCreate(NULL, &wi);
        gdispImageOpenMemory(ghImages + i * 2 + 1, EFFECTS[0]->image);
        gwinSetCustomDraw(opaque->btn_effectIndicate[i * 2 + 1], gwinButtonDraw_Image, ghImages + i * 2 + 1);
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
