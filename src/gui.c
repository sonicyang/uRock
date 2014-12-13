#include "gui.h"

void gui_ButtonInit(Button* btn)
{
    btn->x = 0;
    btn->y = 0;
    btn->w = 0;
    btn->h = 0;

    btn->color = LCD_COLOR_BLACK;
    btn->renderType = BUTTON_RENDER_TYPE_LINE;

    btn->cb = NULL;
}

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

void gui_ValueBarInit(ValueBar* bar)
{
    bar->x = 0;
    bar->y = 0;
    bar->w = 0;
    bar->h = 0;

    bar->currentValue = 0;

    bar->outColor = LCD_COLOR_BLACK;
    bar->inColor = LCD_COLOR_GREEN;

    bar->cb = NULL;
}

void gui_ValueBarSetPos(ValueBar* bar, uint16_t x, uint16_t y)
{
    bar->x = x;
    bar->y = y;
}

void gui_ValueBarSetSize(ValueBar* bar, uint16_t w, uint16_t h)
{
    bar->w = w;
    bar->h = h;
}

void gui_ValueBarSetColor(ValueBar* bar, uint32_t inColor, uint32_t outColor)
{
    bar->inColor = inColor;
    bar->outColor = outColor;
}

void gui_ValueBarSetCallback(ValueBar* bar, ValueBarSetValueCallback cb)
{
    bar->cb = cb;
}

void gui_ValueBarRender(ValueBar* bar)
{
    uint32_t storedTextColor;

    storedTextColor = BSP_LCD_GetTextColor();

    BSP_LCD_SetTextColor(bar->outColor);
    BSP_LCD_DrawRect(bar->x, bar->y, bar->w, bar->h);

    BSP_LCD_SetTextColor(bar->inColor);
    if (bar->currentValue > 0)
        BSP_LCD_FillRect((bar->x + 1),
                         (bar->y + 1),
                         map(bar->currentValue, 0, 255, 0, (bar->w - 1)),
                         (bar->h - 1));

    BSP_LCD_SetTextColor(storedTextColor);
}

void gui_ValueBarHandleEvent(ValueBar* bar, uint16_t touchX, uint16_t touchY)
{
    if ((touchX > bar->x) && (touchX < bar->x + bar->w) &&
        (touchY > bar->y) && (touchY < bar->y + bar->h)){

        bar->currentValue = map(touchX - bar->x, 0, bar->w, 0, 255);
    }
}
