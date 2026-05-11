/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : SSD1306 UI DEMO
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "main.h"

#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ssd1306_conf.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim3;

/* ========================= */
/* UI */
/* ========================= */




static bool menu_mode = false;

static uint8_t menu_selected = 0;
static uint8_t menu_scroll = 0;
static uint8_t menuInfo_shift = 0;


static int16_t last_encoder = 0;

static uint16_t fake_voltage = 0;
static uint16_t fake_current = 0;
static uint32_t fake_power   = 0;

#define MENU_ITEMS 32

static char* menu_list[MENU_ITEMS] =
{
    "Diagnostics",
    "Power Monitor",
    "Signal Analyzer",
    "Sensor Matrix",
    "System Status",
    "Factory Reset",
    "ADC Monitor",
    "GPIO Viewer",
    "PWM Generator",
    "UART Console",
    "SPI Devices",
    "I2C Scanner",
    "CAN Bus",
    "EEPROM Tool",
    "RTC Clock",
    "DAC Output",
    "DMA Streams",
    "Bootloader",
    "Memory Viewer",
    "CPU Load",
    "Temperature",
    "Fan Control",
    "Voltage Rails",
    "Spectrum View",
    "Wave Generator",
    "Logic Analyzer",
    "Battery Health",
    "Power Saving",
    "Filesystem",
    "Debug Terminal",
    "Firmware Info",
    "Developer Mode"
};

static char* menuInfo_list[MENU_ITEMS] =
{
    "Dima GAY",
    "Live voltage monitor",
    "FFT / waveform engine",
    "I2C sensor network",
    "Temperature / RAM / CPU",
    "Reset all user settings",
    "Raw ADC live values",
    "GPIO pin state monitor",
    "PWM frequency control",
    "Serial communication",
    "SPI peripheral manager",
    "Search I2C addresses",
    "CAN packet analyzer",
    "EEPROM read/write tool",
    "Realtime clock settings",
    "Analog output generator",
    "DMA transfer monitor",
    "Firmware update utility",
    "RAM / FLASH browser",
    "CPU usage statistics",
    "MCU thermal monitor",
    "Cooling system control",
    "Power line telemetry",
    "Frequency spectrum mode",
    "Signal waveform output",
    "Digital signal capture",
    "Battery diagnostics",
    "Low power management",
    "Internal file browser",
    "Embedded debug shell",
    "Firmware build details",
    "Advanced engineering tools"
};

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);



static void draw_menu_info(void)
{
    static uint8_t menuSelected = 255;

    static uint8_t offset = 0;
    static uint8_t delay = 0;

    static const char* text;
    static char scrollBuf[128]; 
    static uint16_t len;

    /* ========================= */
    /* смена пункта */
    /* ========================= */

    if (menuSelected != menu_selected)
    {
        menuSelected = menu_selected;

        offset = 0;
        delay = 0;

        text = menuInfo_list[menu_selected];

        /* формируем буфер ОДИН РАЗ */
        snprintf(scrollBuf, sizeof(scrollBuf),
            "%s   %s   ",
            text, text);

        len = strlen(scrollBuf);
    }


    
    /* ========================= */
    /* линия */
    /* ========================= */

    ssd1306_Line(0, 50, 127, 50, White);

    /* ========================= */
    /* задержка перед скроллом */
    /* ========================= */

    if (delay < 15)
    {
        delay++;
    }
    else
    {
        offset += 1;
        if (offset >= len)
            offset = 0;
    }

    /* ========================= */
    /* ОКНО (видимая часть строки) */
    /* ========================= */

    ssd1306_SetCursor(4, 54);

    /* ========================= */
    /* РИСУЕМ ОКНО СКРОЛЛА */
    /* ========================= */
    for (uint8_t i = 0; i < 21; i++) // ~21 символ помещается
    {
        char c = scrollBuf[(offset + i) % len];
        ssd1306_WriteChar(c, Font_6x8, White);
    }
    
}




/* ========================= */
/* RANDOM DATA */
/* ========================= */

static void generate_fake_data(void)
{
    fake_voltage =
        2100 + (rand() % 400);

    fake_current =
        100 + (rand() % 900);

    fake_power =
        fake_voltage * fake_current;
}

/* ========================= */
/* SIDE BARS */
/* ========================= */

static void draw_side_bars(void)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t h =
            5 + (rand() % 24);

        ssd1306_FillRectangle(
            100 + (i * 3),
            60 - h,
            101 + (i * 3),
            60,
            White
        );
    }
}

/* ========================= */
/* MAIN DASHBOARD */
/* ========================= */

static void draw_dashboard(void)
{
    char buf[32];

    /* FRAME */

    ssd1306_DrawRectangle(
        0,
        0,
        127,
        63,
        White
    );

    /* HEADER */

    ssd1306_SetCursor(24, 2);

    ssd1306_WriteString(
        "Volt-Amp Meter",
        Font_7x10,
        White
    );

    ssd1306_Line(
        0,
        14,
        127,
        14,
        White
    );

    /* BIG VOLTAGE */

    sprintf(
        buf,
        "%2d.%01dV",
        fake_voltage / 100,
        (fake_voltage / 10) % 10
    );

    ssd1306_SetCursor(18, 18);

    ssd1306_WriteString(
        buf,
        Font_11x18,
        White
    );

    /* CURRENT */

    sprintf(
        buf,
        "%2d.%01dA",
        fake_current / 100,
        (fake_current / 10) % 10
    );

    ssd1306_SetCursor(10, 44);

    ssd1306_WriteString(
        buf,
        Font_7x10,
        White
    );

    /* POWER */

    sprintf(
        buf,
        "%luW",
        fake_power / 100
    );

    ssd1306_SetCursor(10, 54);

    ssd1306_WriteString(
        buf,
        Font_6x8,
        White
    );

    /* STATUS */

    ssd1306_SetCursor(94, 20);

    ssd1306_WriteString(
        "LIVE",
        Font_6x8,
        White
    );

    ssd1306_SetCursor(94, 32);

    ssd1306_WriteString(
        "SYS OK",
        Font_6x8,
        White
    );

    ssd1306_SetCursor(94, 44);

    ssd1306_WriteString(
        "I2C OK",
        Font_6x8,
        White
    );

    /* CIRCLE */

    ssd1306_DrawCircle(
        15,
        28,
        6,
        White
    );

    ssd1306_FillCircle(
        15,
        28,
        2,
        White
    );

    /* SIDE BARS */

    draw_side_bars();

/* animated bottom line */

for (uint8_t i = 0; i < 127; i += 4)
{
    uint8_t y =
        63 -
        ((i + HAL_GetTick() / 8) % 6);

    ssd1306_DrawPixel(
        i,
        y,
        White
    );
}


}
static void draw_menu(void)
{
    ssd1306_DrawRectangle(
        0,
        0,
        127,
        63,
        White
    );

    /* HEADER */

    ssd1306_SetCursor(24, 2);

    ssd1306_WriteString(
        "CONTROL MENU",
        Font_7x10,
        White
    );

    ssd1306_Line(
        0,
        14,
        127,
        14,
        White
    );

    /* ========================= */
    /* SCROLL LOGIC */
    /* ========================= */

    if (menu_selected < menu_scroll)
    {
        menu_scroll = menu_selected;
    }

    if (menu_selected >= (menu_scroll + 4))
    {
        menu_scroll =
            menu_selected - 3;
    }

    /* ========================= */
    /* DRAW VISIBLE ITEMS */
    /* ========================= */

    for (uint8_t i = 0; i < 4; i++)
    {
        uint8_t item_index =
            menu_scroll + i;

        if (item_index >= MENU_ITEMS)
            break;

        uint8_t y =
            18 + (i * 8);

        /* SELECTED */

        if (item_index == menu_selected)
        {
            ssd1306_FillRectangle(
                2,
                y - 1,
                118,
                y + 7,
                White
            );

            ssd1306_FillCircle(
                7,
                y +3,
                1,
                Black
            );

            ssd1306_SetCursor(
                12,
                y
            );

            ssd1306_WriteString(
                menu_list[item_index],
                Font_6x8,
                Black
            );
        }
        else
        {
            ssd1306_SetCursor(
                12,
                y
            );

            ssd1306_WriteString(
                menu_list[item_index],
                Font_6x8,
                White
            );
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
        White
    );

    uint8_t scroll_h =
        8;

    uint8_t scroll_y =
        19 +
        ((20 * menu_selected)
        / (MENU_ITEMS - 1));

    ssd1306_FillRectangle(
        122,
        scroll_y,
        124,
        scroll_y + scroll_h,
        White
    );

    /* ========================= */
    /* INFO */
    /* ========================= */

    draw_menu_info();
}

/* ========================= */
/* MAIN */
/* ========================= */

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM3_Init();

    HAL_Delay(100);

    ssd1306_Init();

    ssd1306_Fill(Black);

    ssd1306_UpdateScreen();

    HAL_TIM_Encoder_Start(
        &htim3,
        TIM_CHANNEL_ALL
    );

    __HAL_TIM_SET_COUNTER(
        &htim3,
        0
    );

    while (1)
    {
        /* ========================= */
        /* BUTTON */
        /* ========================= */

        static bool old_button = false;

        bool button_pressed =
            (HAL_GPIO_ReadPin(
                GPIOA,
                GPIO_PIN_5
            ) == GPIO_PIN_RESET);

        if (button_pressed && !old_button)
        {
            menu_mode = !menu_mode;

            HAL_Delay(150);
        }

        old_button = button_pressed;

        /* ========================= */
        /* ENCODER */
        /* ========================= */

        int16_t encoder_now =
            __HAL_TIM_GET_COUNTER(&htim3);

        if (encoder_now != last_encoder)
        {
            if (menu_mode)
            {
                if (encoder_now > last_encoder)
                {
                    if (menu_selected < (MENU_ITEMS - 1))
                    {
                        menu_selected++;
                    }
                }
                else
                {
                    if (menu_selected > 0)
                    {
                        menu_selected--;
                    }
                }
            }

            last_encoder = encoder_now;
        }

        /* ========================= */
        /* DATA */
        /* ========================= */

        generate_fake_data();

        /* ========================= */
        /* DRAW */
        /* ========================= */

        ssd1306_Fill(Black);

        if (menu_mode)
        {
            draw_menu();
        }
        else
        {
            draw_dashboard();
        }

        ssd1306_UpdateScreen();

        HAL_Delay(40);
    }
}

/* ========================= */
/* CLOCK */
/* ========================= */

void SystemClock_Config(void)
{
}

/* ========================= */
/* I2C */
/* ========================= */

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;

    hi2c1.Init.Timing = 0x00910B1C;

    hi2c1.Init.OwnAddress1 = 0;

    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;

    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;

    hi2c1.Init.OwnAddress2 = 0;

    hi2c1.Init.OwnAddress2Masks =
        I2C_OA2_NOMASK;

    hi2c1.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;

    hi2c1.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c1);

    HAL_I2CEx_ConfigAnalogFilter(
        &hi2c1,
        I2C_ANALOGFILTER_ENABLE
    );

    HAL_I2CEx_ConfigDigitalFilter(
        &hi2c1,
        0
    );
}

/* ========================= */
/* TIM3 */
/* ========================= */

static void MX_TIM3_Init(void)
{
    TIM_Encoder_InitTypeDef sConfig = {0};

    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim3.Instance = TIM3;

    htim3.Init.Prescaler = 0;

    htim3.Init.CounterMode =
        TIM_COUNTERMODE_UP;

    htim3.Init.Period = 65535;

    htim3.Init.ClockDivision =
        TIM_CLOCKDIVISION_DIV1;

    htim3.Init.AutoReloadPreload =
        TIM_AUTORELOAD_PRELOAD_DISABLE;

    sConfig.EncoderMode =
        TIM_ENCODERMODE_TI1;

    sConfig.IC1Polarity =
        TIM_ICPOLARITY_RISING;

    sConfig.IC1Selection =
        TIM_ICSELECTION_DIRECTTI;

    sConfig.IC1Prescaler =
        TIM_ICPSC_DIV1;

    sConfig.IC1Filter = 10;

    sConfig.IC2Polarity =
        TIM_ICPOLARITY_RISING;

    sConfig.IC2Selection =
        TIM_ICSELECTION_DIRECTTI;

    sConfig.IC2Prescaler =
        TIM_ICPSC_DIV1;

    sConfig.IC2Filter = 10;

    HAL_TIM_Encoder_Init(
        &htim3,
        &sConfig
    );

    sMasterConfig.MasterOutputTrigger =
        TIM_TRGO_RESET;

    sMasterConfig.MasterSlaveMode =
        TIM_MASTERSLAVEMODE_DISABLE;

    HAL_TIMEx_MasterConfigSynchronization(
        &htim3,
        &sMasterConfig
    );
}

/* ========================= */
/* GPIO */
/* ========================= */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* BUTTON */

    GPIO_InitStruct.Pin =
        GPIO_PIN_5;

    GPIO_InitStruct.Mode =
        GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull =
        GPIO_PULLUP;

    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );
}

/* ========================= */
/* ERROR */
/* ========================= */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}