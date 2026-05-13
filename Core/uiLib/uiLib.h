#ifndef __UILIB_H__
#define __UILIB_H__

#include <stddef.h>

#define MENU_ITEMS 6
#define MAX_STACK 8

_BEGIN_STD_C

typedef struct MenuItem
{
    const char *name;
    const char *description;

    struct MenuItem *parent;
    struct MenuItem *children;

    uint8_t childrenCount;

    void (*onSelect)(void);
} MenuItem_t;

typedef struct
{
    MenuItem_t *items;
    uint8_t itemsCount;

    MenuItem_t *stack[MAX_STACK];
    uint8_t stackDepth;

    uint8_t selected;
    uint8_t scroll;

} UiMenu_t;

extern UiMenu_t ui;

void uiLib_init(MenuItem_t *items, uint8_t count);
void uiLib_renderMenu(void);

_END_STD_C

#endif /* __UILIB_H__ */