#include "tab_select_effect.h"

#include "base-effect.h"

#include "gfxconf.h"
#include "gfx.h"

#include "setting.h"
#include "ui.h"

extern uint32_t selectedEffectStage;

void tab_select_effect_show(void* opaque){
	struct tab_select_effect_t *tmp = (struct tab_select_effect_t*)opaque;
	uint32_t i;

	for(i = 0; i < EFFECT_TYPE_NUM; i++){
		gwinSetVisible(tmp->btn_effectTypes[i], TRUE);
	}

	return;
}

void tab_select_effect_hide(void* opaque){
	struct tab_select_effect_t *tmp = (struct tab_select_effect_t*)opaque;
	uint32_t i;

	for(i = 0; i < EFFECT_TYPE_NUM; i++){
		gwinSetVisible(tmp->btn_effectTypes[i], FALSE);
	}

	return;
}

void tab_select_effect_refresh(void* opaque){
	return;
}

void tab_select_effect_bHandle(void* opaque, GEventGWinButton* event){
	struct tab_select_effect_t *tmp = (struct tab_select_effect_t*)opaque;
	uint32_t i;

	for(i = 0; i < EFFECT_TYPE_NUM; i++){
		if ((event)->gwin == tmp->btn_effectTypes[i]){
            attachEffect(selectedEffectStage, i);
			SwitchTab(LIST_TAB);
		}
	}

	if ((event)->gwin == tmp->btn_effectTypes[EFFECT_TYPE_NUM -1]){
		SwitchTab(LIST_TAB);
	}

	return;
}

struct tab_t *tab_select_effect_init(struct tab_select_effect_t* opaque){
	uint32_t i;
	GWidgetInit wi;

	/* SelectStageTab */
	for(i = 0; i < EFFECT_TYPE_NUM - 1; i++){
		gwinWidgetClearInit(&wi);
		wi.g.show = FALSE;
		wi.g.x = 5;
		wi.g.y = 5 + 25 * i;
		wi.g.width = 230;
		wi.g.height = 20;
		wi.text = cvtToEffectName(i);
		opaque->btn_effectTypes[i] = gwinButtonCreate(NULL, &wi);
	}

	gwinWidgetClearInit(&wi);
	wi.g.show = FALSE;
	wi.g.x = 5;
	wi.g.y = 290;
	wi.g.width = 230;
	wi.g.height = 20;
	wi.text = "Back";
	opaque->btn_effectTypes[EFFECT_TYPE_NUM - 1] = gwinButtonCreate(NULL, &wi);

	opaque->parent.show = tab_select_effect_show;
	opaque->parent.hide = tab_select_effect_hide;
	opaque->parent.refresh = tab_select_effect_refresh;
	opaque->parent.bHandle = tab_select_effect_bHandle;

	return (struct tab_t*)opaque;
}

