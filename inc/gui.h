#ifndef __GUI_H__
#define __GUI_H__

#include "stm32f429i_discovery_lcd.h"

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

void gui_ButtonSetPos(Button* btn, uint16_t x, uint16_t y);
void gui_ButtonSetSize(Button* btn, uint16_t w, uint16_t h);
void gui_ButtonSetColor(Button* btn, uint32_t color);
void gui_ButtonSetRenderType(Button* btn, uint8_t type);
void gui_ButtonSetCallback(Button* btn, ButtonPressCallback cb);
void gui_ButtonRender(Button* btn);
void gui_ButtonHandleEvent(Button* btn, uint16_t touchX, uint16_t touchY);

#endif /* __GUI_H__ */
