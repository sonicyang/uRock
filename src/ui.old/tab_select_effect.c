#include "tab_select_effect.h"

#include "base-effect.h"

#include "gfxconf.h"
#include "gfx.h"

#include "setting.h"
#include "ui.h"

static char *cvtToEffectName(uint32_t ee){
	switch(ee){
	case VOL:
		return "Volume";
	case COMP:
		return "Compressor";
	case DISTOR:
		return "Distortion";
	case OVERDR:
		return "OverDrive";
	case DELAY:
		return "Delay";
	case REVERB:
		return "Reverb";
	case FLANGE:
		return "Flanger";
	case EQULIZ:
		return "Equalizer";
	default:
		return "None";
	}
	return "None";
}

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

void tab_select_effect_eHandle(void* opaque, GEventGWinButton* event){
	struct tab_select_effect_t *tmp = (struct tab_select_effect_t*)opaque;
	uint32_t i;
/*
	for(i = 0; i < EFFECT_TYPE_NUM - 1; i++){
		if ((event)->button == tmp->btn_effectTypes[i]){
			StageEffectSelect(i);
			SwitchTab(LIST_TAB);
		}
	}

	if ((event)->button == tmp->btn_effectTypes[EFFECT_TYPE_NUM -1]){
		SwitchTab(LIST_TAB);
	}
*/
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
	opaque->parent.eHandle = tab_select_effect_eHandle;

	return (struct tab_t*)opaque;
}

