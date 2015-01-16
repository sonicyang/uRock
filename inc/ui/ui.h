#ifndef __UI_H__
#define __UI_H__

extern uint8_t currentConfig;

enum{
	LIST_TAB,
	PARAM_TAB,
	SELECT_EFFECT_TAB,
	TAB_NUM
};

void UserInterface(void *argument);
void SwitchTab(uint32_t tab);
void StageEffectSelect(uint8_t whichEffect);

#endif //__UI_H__
