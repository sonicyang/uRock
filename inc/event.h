#ifndef __EVENT_H__
#define __EVENT_H__

enum EventType{
    TP_PRESSED = 0x00000000,
    TP_RELEASED
};

typedef struct{
    enum EventType eventType;
    uint16_t touchX;
    uint16_t touchY;
}Event;

#endif /* __EVENT_H__ */
