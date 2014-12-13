#include "gui.h"

void gui_ButtonSetPos(Button* btn, uint16_t x, uint16_t y)
{
    btn->x = x;
    btn->y = y;
}

void gui_ButtonSetSize(Button* btn, uint16_t w, uint16_t h)
{
    btn->w = w;
    btn->h = h;
}

void gui_ButtonSetColor(Button* btn, uint32_t color)
{
    btn->color = color;
}

void gui_ButtonSetRenderType(Button* btn, uint8_t type)
{
    btn->renderType = type;
}

void gui_ButtonSetCallback(Button* btn, ButtonPressCallback cb)
{
    btn->cb = cb;
}

void gui_ButtonRender(Button* btn)
{
    uint32_t storedTextColor;

    storedTextColor = BSP_LCD_GetTextColor();
    BSP_LCD_SetTextColor(btn->color);

    switch(btn->renderType){
    case BUTTON_RENDER_TYPE_LINE:
        BSP_LCD_DrawRect(btn->x, btn->y, btn->w, btn->h);
        break;
    case BUTTON_RENDER_TYPE_FILL:
        BSP_LCD_FillRect(btn->x, btn->y, btn->w, btn->h);
        break;
    }

    BSP_LCD_SetTextColor(storedTextColor);
}

void gui_ButtonHandleEvent(Button* btn, uint16_t touchX, uint16_t touchY)
{
    if ((touchX > btn->x) && (touchX < btn->x + btn->w) &&
        (touchY > btn->y) && (touchY < btn->y + btn->h)){

        btn->cb();
    }
}
