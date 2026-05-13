#include <ssd1306.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "uiLib.h"
#include "ssd1306_fonts.h"

UiMenu_t ui;

/* ===================================================== */
/* INIT */
/* ===================================================== */

void uiLib_init(MenuItem_t *items, uint8_t count)
{
    ui.items = items;
    ui.itemsCount = count;

    ui.selected = 0;
    ui.scroll = 0;
}

/* ===================================================== */
/* DRAW INFO */
/* ===================================================== */

static void draw_menu_info(void)
{
    static uint8_t last_selected = 255;

    static uint8_t offset = 0;
    static uint8_t delay = 0;

    static const char *text;
    static char scrollBuf[128];
    static uint16_t len;

    if (last_selected != ui.selected)
    {
        last_selected = ui.selected;

        offset = 0;
        delay = 0;

        text =
            ui.items[ui.selected].description;

        if (text == NULL)
            return;

        snprintf(
            scrollBuf,
            sizeof(scrollBuf),
            "%s    %s    ",
            text,
            text);

        len = strlen(scrollBuf);
    }

    ssd1306_Line(0, 50, 127, 50, White);

    if (delay < 15)
    {
        delay++;
    }
    else
    {
        offset++;

        if (offset >= len)
            offset = 0;
    }

    ssd1306_SetCursor(4, 54);

    for (uint8_t i = 0; i < 21; i++)
    {
        char c =
            scrollBuf[(offset + i) % len];

        ssd1306_WriteChar(
            c,
            Font_6x8,
            White);
    }
}

/* ===================================================== */
/* RENDER MENU */
/* ===================================================== */

void uiLib_renderMenu(void)
{
    if (ui.items == NULL)
        return;

    ssd1306_Fill(Black);

    ssd1306_DrawRectangle(
        0,
        0,
        127,
        63,
        White);

    /* ========================= */
    /* HEADER */
    /* ========================= */

    ssd1306_SetCursor(24, 2);

    ssd1306_WriteString(
        "CONTROL MENU",
        Font_7x10,
        White);

    ssd1306_Line(
        0,
        14,
        127,
        14,
        White);

    /* ========================= */
    /* SCROLL LOGIC */
    /* ========================= */

    if (ui.selected < ui.scroll)
    {
        ui.scroll = ui.selected;
    }

    if (ui.selected >= (ui.scroll + 4))
    {
        ui.scroll =
            ui.selected - 3;
    }

    /* ========================= */
    /* DRAW ITEMS */
    /* ========================= */

    for (uint8_t i = 0; i < 4; i++)
    {
        uint8_t item_index =
            ui.scroll + i;

        if (item_index >= ui.itemsCount)
            break;

        uint8_t y =
            18 + (i * 8);

        MenuItem_t *child =
            &ui.items[item_index];

        const char *name =
            child->name;

        if (name == NULL)
            name = "";

        /* ========================= */
        /* SELECTED */
        /* ========================= */

        if (item_index == ui.selected)
        {
            ssd1306_FillRectangle(
                2,
                y - 1,
                118,
                y + 7,
                White);

            ssd1306_FillCircle(
                7,
                y + 3,
                1,
                Black);

            ssd1306_SetCursor(
                12,
                y);

            ssd1306_WriteString(
                (char *)name,
                Font_6x8,
                Black);
        }
        else
        {
            ssd1306_SetCursor(
                12,
                y);

            ssd1306_WriteString(
                (char *)name,
                Font_6x8,
                White);
        }
    }

    /* ========================= */
    /* SCROLLBAR */
    /* ========================= */

    ssd1306_DrawRectangle(
        121,
        18,
        125,
        48,
        White);

    uint8_t scroll_h = 8;

    uint8_t scroll_y = 19;

    if (ui.itemsCount > 1)
    {
        scroll_y =
            19 +
            ((20 * ui.selected) /
             (int)(ui.itemsCount - 1));
    }

    ssd1306_FillRectangle(
        122,
        scroll_y,
        124,
        scroll_y + scroll_h,
        White);

    /* ========================= */
    /* INFO */
    /* ========================= */

    const char *desc =
        ui.items[ui.selected].description;

    if (desc == NULL || desc[0] == '\0')
        return;

    draw_menu_info();
}