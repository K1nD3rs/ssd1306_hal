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

#include "../uiLib/uiLib.h"

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

static int16_t last_encoder = 0;

/* ===================================================== */
/* ACTIONS */
/* ===================================================== */

void set_volume(void) {};
void toggle_mute(void) {};
void show_info(void) {};
void adc_monitor(void) {};
void gpio_viewer(void) {};
void pwm_generator(void) {};
void uart_console(void) {};
void spi_devices(void) {};
void i2c_scanner(void) {};
void can_bus(void) {};
void rtc_clock(void) {};
void battery_health(void) {};
void developer_mode(void) {};
void factory_reset(void) {};

/* ===================================================== */
/* DEBUG MENU */
/* ===================================================== */

MenuItem_t debugMenu[] =
    {
        {"UART Console",
         "Serial communication",
         NULL,
         NULL,
         0,
         uart_console},

        {"GPIO Viewer",
         "GPIO pin state monitor",
         NULL,
         NULL,
         0,
         gpio_viewer},

        {"ADC Monitor",
         "Raw ADC live values",
         NULL,
         NULL,
         0,
         adc_monitor},

        {"Developer Mode",
         "Advanced engineering tools",
         NULL,
         NULL,
         0,
         developer_mode}};

/* ===================================================== */
/* POWER MENU */
/* ===================================================== */

MenuItem_t powerMenu[] =
    {
        {"Battery Health",
         "Battery diagnostics",
         NULL,
         NULL,
         0,
         battery_health},

        {"Power Saving",
         "Low power management",
         NULL,
         NULL,
         0,
         NULL},

        {"Voltage Rails",
         "Power line telemetry",
         NULL,
         NULL,
         0,
         NULL},

        {"Fan Control",
         "Cooling system control",
         NULL,
         NULL,
         0,
         NULL}};

/* ===================================================== */
/* INTERFACES MENU */
/* ===================================================== */

MenuItem_t interfacesMenu[] =
    {
        {"UART Console",
         "Serial communication",
         NULL,
         NULL,
         0,
         uart_console},

        {"SPI Devices",
         "SPI peripheral manager",
         NULL,
         NULL,
         0,
         spi_devices},

        {"I2C Scanner",
         "Search I2C addresses",
         NULL,
         NULL,
         0,
         i2c_scanner},

        {"CAN Bus",
         "CAN packet analyzer",
         NULL,
         NULL,
         0,
         can_bus}};

/* ===================================================== */
/* SOUND MENU */
/* ===================================================== */

MenuItem_t soundMenu[] =
    {
        {"Volume",
         "Set volume",
         NULL,
         NULL,
         0,
         set_volume},

        {"Mute",
         "Toggle mute",
         NULL,
         NULL,
         0,
         toggle_mute}};

/* ===================================================== */
/* SYSTEM MENU */
/* ===================================================== */

MenuItem_t systemMenu[] =
    {
        {"System Status",
         "Temperature / RAM / CPU",
         NULL,
         NULL,
         0,
         NULL},

        {"Firmware Info",
         "Firmware build details",
         NULL,
         NULL,
         0,
         show_info},

        {"RTC Clock",
         "Realtime clock settings",
         NULL,
         NULL,
         0,
         rtc_clock},

        {"Factory Reset",
         "Reset all user settings",
         NULL,
         NULL,
         0,
         factory_reset}};

/* ===================================================== */
/* TOOLS MENU */
/* ===================================================== */

MenuItem_t toolsMenu[] =
    {
        {"Signal Analyzer",
         "FFT / waveform engine",
         NULL,
         NULL,
         0,
         NULL},

        {"Logic Analyzer",
         "Digital signal capture",
         NULL,
         NULL,
         0,
         NULL},

        {"Wave Generator",
         "Signal waveform output",
         NULL,
         NULL,
         0,
         NULL},

        {"PWM Generator",
         "PWM frequency control",
         NULL,
         NULL,
         0,
         pwm_generator}};

/* ===================================================== */
/* MAIN MENU */
/* ===================================================== */

MenuItem_t mainMenu[] =
    {
        {"Sound",
         "Audio settings",
         NULL,
         soundMenu,
         sizeof(soundMenu) / sizeof(MenuItem_t),
         NULL},

        {"System",
         "System tools and info",
         NULL,
         systemMenu,
         sizeof(systemMenu) / sizeof(MenuItem_t),
         NULL},

        {"Power",
         "Power management",
         NULL,
         powerMenu,
         sizeof(powerMenu) / sizeof(MenuItem_t),
         NULL},

        {"Interfaces",
         "Communication peripherals",
         NULL,
         interfacesMenu,
         sizeof(interfacesMenu) / sizeof(MenuItem_t),
         NULL},

        {"Tools",
         "Signal processing utilities",
         NULL,
         toolsMenu,
         sizeof(toolsMenu) / sizeof(MenuItem_t),
         NULL},

        {"Debug",
         "Low level debug functions",
         NULL,
         debugMenu,
         sizeof(debugMenu) / sizeof(MenuItem_t),
         NULL}};

uint8_t Button_Handler(void);

/* MenuItem_t soundMenu[] =
    {
        {"Volume", "Set volume", NULL, NULL, 0, set_volume},
        {"Mute", "Toggle mute", NULL, NULL, 0, toggle_mute}};

MenuItem_t mainMenu[] =
    {
        {"Sound", "Audio settings", NULL, soundMenu, 2, NULL},
        {"Info", "System info", NULL, NULL, 0, show_info}}; */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);

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
        TIM_CHANNEL_ALL);

    __HAL_TIM_SET_COUNTER(
        &htim3,
        0);

    uiLib_init(mainMenu, 6);

    while (1)
    {

        switch (Button_Handler())
        {
        case 1:
            MenuItem_t *current =
                &ui.items[ui.selected];

            // если есть подменю
            if (current->children != NULL)
            {
                ui.stack[ui.stackDepth++] = ui.items;

                ui.items = current->children;
                ui.itemsCount = current->childrenCount;

                ui.selected = 0;
                ui.scroll = 0;
            }
            break;

        case 2:
            if (ui.stackDepth > 0)
            {
                ui.items = ui.stack[--ui.stackDepth];
                // ui.itemsCount = ui.items->childrenCount;
            }

            break;
        }

        /* ========================= */
        /* ENCODER */
        /* ========================= */

        int16_t encoder_now =
            __HAL_TIM_GET_COUNTER(&htim3);

        if (encoder_now != last_encoder)
        {

            if (encoder_now > last_encoder)
            {
                if (ui.selected < (MENU_ITEMS - 1))
                {
                    ui.selected++;
                }
            }
            else
            {
                if (ui.selected > 0)
                {
                    ui.selected--;
                }
            }

            last_encoder = encoder_now;
        }

        /* ========================= */
        /* DRAW */
        /* ========================= */

        uiLib_renderMenu();
        ssd1306_UpdateScreen(); // ! Отрисовка буффера
        HAL_Delay(5);
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
        I2C_ANALOGFILTER_ENABLE);

    HAL_I2CEx_ConfigDigitalFilter(
        &hi2c1,
        0);
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
        &sConfig);

    sMasterConfig.MasterOutputTrigger =
        TIM_TRGO_RESET;

    sMasterConfig.MasterSlaveMode =
        TIM_MASTERSLAVEMODE_DISABLE;

    HAL_TIMEx_MasterConfigSynchronization(
        &htim3,
        &sMasterConfig);
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
        &GPIO_InitStruct);
}

/* ========================= */
/* BUTTON_HANDLER */
/* ========================= */

uint8_t Button_Handler(void)
{
    static uint32_t lastClick = 0;
    static uint8_t clickCount = 0;

    if (!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
    {
        HAL_Delay(20); // антидребезг

        if (!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
        {
            while (!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
                ;

            clickCount++;

            if (HAL_GetTick() - lastClick > 300)
            {
                clickCount = 1;
            }

            lastClick = HAL_GetTick();
        }
    }

    if (clickCount > 0 && (HAL_GetTick() - lastClick) > 300)
    {
        uint8_t result = clickCount;
        clickCount = 0;
        return result;
    }

    return 255; // ничего
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