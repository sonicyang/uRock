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

void gui_ButtonHandleEvent(Button* btn, Event* event)
{
    switch(event->eventType){
    case TP_PRESSED:
        if ((event->touchX > btn->x) && (event->touchX < btn->x + btn->w) &&
            (event->touchY > btn->y) && (event->touchY < btn->y + btn->h)){

            btn->cb();
        }
        break;
    case TP_RELEASED:
        break;
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

    bar->cbSet = NULL;
    bar->cbGet = NULL;
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

void gui_ValueBarSetCallbacks(ValueBar* bar, ValueBarSetValueCallback cbSet,
                              ValueBarGetValueCallback cbGet)
{
    bar->cbSet = cbSet;
    bar->cbGet = cbGet;
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

void gui_ValueBarHandleEvent(ValueBar* bar, Event* event)
{
    if (bar->cbGet)
        bar->currentValue = bar->cbGet();

    if ((event->eventType == TP_PRESSED) &&
        (event->touchX > bar->x) && (event->touchX < bar->x + bar->w) &&
        (event->touchY > bar->y) && (event->touchY < bar->y + bar->h)){

        bar->currentValue = map(event->touchX - bar->x, 0, bar->w, 0, 255);

        if (bar->cbSet)
            bar->cbSet(bar->currentValue);
    }
}
