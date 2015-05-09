#ifndef __TAB_BASE_H__
#define __TAB_BASE_H__

#include "gfxconf.h"
#include "gfx.h"

typedef void(*ShowFunc)(void *opaque);
typedef void(*HideFunc)(void *opaque);
typedef void(*RefreshFunc)(void *opaque);
typedef void(*EventHandleFunc)(void *opaque, GEventGWinButton* event);

struct tab_t{
    ShowFunc show;
    HideFunc hide;
    RefreshFunc refresh;
    EventHandleFunc eHandle;
};

#endif //__TAB_LIST_H__
