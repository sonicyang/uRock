#ifndef __GUI_H__
#define __GUI_H__

#include "stm32f429i_discovery_lcd.h"

#include "event.h"
#include "helper.h"

#define BUTTON_RENDER_TYPE_LINE 0x00
#define BUTTON_RENDER_TYPE_FILL 0x01

typedef void(*ButtonPressCallback)();
typedef struct{
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;

    uint32_t color;
    uint8_t renderType;

    ButtonPressCallback cb;
}Button;

void gui_ButtonInit(Button* btn);
void gui_ButtonSetPos(Button* btn, uint16_t x, uint16_t y);
void gui_ButtonSetSize(Button* btn, uint16_t w, uint16_t h);
void gui_ButtonSetColor(Button* btn, uint32_t color);
void gui_ButtonSetRenderType(Button* btn, uint8_t type);
void gui_ButtonSetCallback(Button* btn, ButtonPressCallback cb);
void gui_ButtonRender(Button* btn);
void gui_ButtonHandleEvent(Button* btn, Event* event);

typedef void(*ValueBarSetValueCallback)(uint8_t value);
typedef struct{
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;

    uint16_t currentValue;

    uint32_t outColor;
    uint32_t inColor;

    ValueBarSetValueCallback cb;
}ValueBar;

void gui_ValueBarInit(ValueBar* bar);
void gui_ValueBarSetPos(ValueBar* bar, uint16_t x, uint16_t y);
void gui_ValueBarSetSize(ValueBar* bar, uint16_t w, uint16_t h);
void gui_ValueBarSetColor(ValueBar* bar, uint32_t inColor, uint32_t outColor);
void gui_ValueBarSetCallback(ValueBar* bar, ValueBarSetValueCallback cb);
void gui_ValueBarRender(ValueBar* bar);
void gui_ValueBarHandleEvent(ValueBar* bar, Event* event);

#endif /* __GUI_H__ */
