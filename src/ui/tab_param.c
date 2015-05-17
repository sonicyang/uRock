#include "tab_param.h"

#include "spu.h"
#include "ui.h"

#include "gfxconf.h"
#include "gfx.h"

#include "setting.h"

#include "helper.h"

extern uint32_t selectedEffectStage;

void tab_param_show(void* opaque){
    struct tab_param_t *tmp = (struct tab_param_t*)opaque;
    struct parameter_t *parameterList[MAX_EFFECT_PARAM];
    uint8_t paraNum;
    char buf[32];
    uint32_t i = 0;

    gwinSetVisible(tmp->label_effectTitle, TRUE);
    for(i = 0; i < MAX_EFFECT_PARAM; i++){
        gwinSetVisible(tmp->label_param[i], FALSE);
        gwinSetVisible(tmp->vbar_param[i], FALSE); //Let Refresh do the job
    }

    gwinSetVisible(tmp->btn_back, TRUE);
    gwinSetVisible(tmp->btn_change, TRUE);

    strcpy(buf, "Stage ");
    itoa(selectedEffectStage + 1, buf + 6);
    strcat(buf, " : ");

    if (retriveStagedEffect(selectedEffectStage)){
        strcat(buf, retriveStagedEffect(selectedEffectStage)->name);
        gwinSetText(tmp->label_effectTitle, buf, 1);

        retriveStagedEffect(selectedEffectStage)->getParam((void*)retriveStagedEffect(selectedEffectStage), parameterList, &paraNum);
        for(;i < paraNum; i++){
            gwinSetVisible(tmp->label_param[i], TRUE);
            gwinSetText(tmp->label_param[i], parameterList[i]->name, 0);

            gwinSetVisible(tmp->vbar_param[i], TRUE);
            ftoa(parameterList[i]->value, buf, 2);
            gwinSetText(tmp->vbar_param[i], buf, 1);
            gwinProgressbarSetPosition(tmp->vbar_param[i], map(parameterList[i]->value, parameterList[i]->lowerBound, parameterList[i]->upperBound, 0, 100));
        }
        i = paraNum;
    }else{
        strcat(buf, "Empty");
        gwinSetText(tmp->label_effectTitle, buf, 1);
    }

    for(; i < MAX_EFFECT_PARAM; i++){
        gwinSetVisible(tmp->vbar_param[i], FALSE);
        gwinSetVisible(tmp->label_param[i], FALSE);
    }

    return;

    return;
}

void tab_param_hide(void* opaque){
    struct tab_param_t *tmp = (struct tab_param_t*)opaque;
    uint32_t i;

    gwinSetVisible(tmp->label_effectTitle, FALSE);
    for(i = 0; i < MAX_EFFECT_PARAM; i++){
        gwinSetVisible(tmp->label_param[i], FALSE);
        gwinSetVisible(tmp->vbar_param[i], FALSE);
    }
    
    gwinSetVisible(tmp->btn_back, FALSE);
    gwinSetVisible(tmp->btn_change, FALSE);

    return;
}

void tab_param_refresh(void* opaque){
    struct tab_param_t *tmp = (struct tab_param_t*)opaque;
    struct parameter_t *parameterList[MAX_EFFECT_PARAM];
    uint8_t paraNum;
    char buf[32];
    uint32_t i = 0;

    strcpy(buf, "Stage ");
    itoa(selectedEffectStage + 1, buf + 6);
    strcat(buf, " : ");

    if (retriveStagedEffect(selectedEffectStage)){
        strcat(buf, retriveStagedEffect(selectedEffectStage)->name);
        gwinSetText(tmp->label_effectTitle, buf, 1);

        retriveStagedEffect(selectedEffectStage)->getParam((void*)retriveStagedEffect(selectedEffectStage), parameterList, &paraNum);
        for(;i < paraNum; i++){
            gwinSetVisible(tmp->label_param[i], TRUE);
            gwinSetText(tmp->label_param[i], parameterList[i]->name, 0);

            gwinSetVisible(tmp->vbar_param[i], TRUE);
            ftoa(parameterList[i]->value, buf, 2);
            gwinSetText(tmp->vbar_param[i], buf, 1);
            gwinProgressbarSetPosition(tmp->vbar_param[i], map(parameterList[i]->value, parameterList[i]->lowerBound, parameterList[i]->upperBound, 0, 100));
        }
        i = paraNum;
    }else{
        strcat(buf, "Empty");
        gwinSetText(tmp->label_effectTitle, buf, 1);
    }

    for(; i < MAX_EFFECT_PARAM; i++){
        gwinSetVisible(tmp->vbar_param[i], FALSE);
        gwinSetVisible(tmp->label_param[i], FALSE);
    }

    return;
}

void tab_param_bHandle(void* opaque, GEventGWinButton* event){
    struct tab_param_t *tmp = (struct tab_param_t*)opaque;

    if ((event)->gwin == tmp->btn_back){
        SwitchTab(LIST_TAB);
    }else if ((event)->gwin == tmp->btn_change){
        SwitchTab(SELECT_EFFECT_TAB);
    }
    return;
}

struct tab_t *tab_param_init(struct tab_param_t* opaque){
    uint32_t i;
	GWidgetInit wi;

    /* ParamTab */
	gwinWidgetClearInit(&wi);
	wi.g.show = FALSE;
	wi.g.x = 5;
	wi.g.y = 10;
	wi.g.width = (240 - 20);
	wi.g.height = 25;
	wi.text = "";
	opaque->label_effectTitle = gwinLabelCreate(NULL, &wi);

    for(i = 0; i < MAX_EFFECT_PARAM; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 5;
        wi.g.y = 50 + i * 55;
        wi.g.width = (240 - 10);
        wi.g.height = 20;
        wi.text = "";
        opaque->label_param[i] = gwinLabelCreate(NULL, &wi);

        wi.g.y += 25;
        opaque->vbar_param[i] = gwinProgressbarCreate(NULL, &wi);
    }

    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    wi.g.x = 130;
    wi.g.y = 260;
    wi.g.width = 100;
    wi.g.height = 45;
    wi.text = "Back";
    opaque->btn_back = gwinButtonCreate(NULL, &wi);

    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    wi.g.x = 10;
    wi.g.y = 260;
    wi.g.width = 100;
    wi.g.height = 45;
    wi.text = "Change";
    opaque->btn_change = gwinButtonCreate(NULL, &wi);

    opaque->parent.show = tab_param_show;
    opaque->parent.hide = tab_param_hide;
    opaque->parent.refresh = tab_param_refresh;
    opaque->parent.bHandle = tab_param_bHandle;

    return (struct tab_t*)opaque;
}
