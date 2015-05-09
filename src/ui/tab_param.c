#include "tab_param.h"

#include "base-effect.h"

#include "gfxconf.h"
#include "gfx.h"
#include "src/gwin/sys_defs.h"

#include "setting.h"

#include "helper.h"

extern struct Effect_t *EffectList[STAGE_NUM];
extern int8_t controllingStage;

void tab_param_show(void* opaque){
    struct tab_param_t *tmp = (struct tab_param_t*)opaque;
    uint32_t i;

    gwinSetVisible(tmp->label_effectTitle, TRUE);
    for(i = 0; i < MAX_EFFECT_PARAM; i++){
        gwinSetVisible(tmp->label_param[i], TRUE);
        gwinSetVisible(tmp->vbar_param[i], TRUE);
    }

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

    return;
}

void tab_param_refresh(void* opaque){
    struct tab_param_t *tmp = (struct tab_param_t*)opaque;
    struct parameter_t *parameterList[5];
    uint8_t paraNum;
    char buf[32];
    uint32_t i;

    if (EffectList[controllingStage]){
        strcpy(buf, "Stage ");
        itoa(controllingStage + 1, buf + 6);
        strcat(buf, " : ");
        strcat(buf, EffectList[controllingStage]->name);
        gwinSetText(tmp->label_effectTitle, buf, 1);

        EffectList[controllingStage]->getParam(EffectList[controllingStage], parameterList, &paraNum);
        for(i = 0; i < paraNum; i++){
            gwinSetVisible(tmp->label_param[i], TRUE);
            gwinSetText(tmp->label_param[i], parameterList[i]->name, 0);

            gwinSetVisible(tmp->vbar_param[i], TRUE);
            ftoa(parameterList[i]->value, buf, 2);
            gwinSetText(tmp->vbar_param[i], buf, 1);
            gwinProgressbarSetPosition(tmp->vbar_param[i], map(parameterList[i]->value, parameterList[i]->lowerBound, parameterList[i]->upperBound, 0, 100));
        }
        i = paraNum;
    }else{
        gwinSetText(tmp->label_effectTitle, "", 0);
    }
    for(; i < MAX_EFFECT_PARAM; i++){
        gwinSetVisible(tmp->vbar_param[i], FALSE);
        gwinSetVisible(tmp->label_param[i], FALSE);
    }

    return;
}

void tab_param_eHandle(void* opaque, GEventGWinButton* event){
    return;
}

struct tab_t *tab_param_init(struct tab_param_t* opaque){
    uint32_t i;
	GWidgetInit wi;

    /* ParamTab */
	gwinWidgetClearInit(&wi);
	wi.g.show = FALSE;
	wi.g.x = 15;
	wi.g.y = 30;
	wi.g.width = (240 - 20);
	wi.g.height = 25;
	wi.text = "";
	opaque->label_effectTitle = gwinLabelCreate(NULL, &wi);

    for(i = 0; i < MAX_EFFECT_PARAM; i++){
        gwinWidgetClearInit(&wi);
        wi.g.show = FALSE;
        wi.g.x = 5;
        wi.g.y = 60 + i * 60;
        wi.g.width = (240 - 10);
        wi.g.height = 25;
        wi.text = "";
        opaque->label_param[i] = gwinLabelCreate(NULL, &wi);

        wi.g.y += 25;
        opaque->vbar_param[i] = gwinProgressbarCreate(NULL, &wi);
    }

    opaque->parent.show = tab_param_show;
    opaque->parent.hide = tab_param_hide;
    opaque->parent.refresh = tab_param_refresh;
    opaque->parent.eHandle = tab_param_eHandle;

    return (struct tab_t*)opaque;
}
