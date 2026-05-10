/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : SSD1306 UI + menu + encoder
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ssd1306_conf.h"

#include <stdbool.h>
#include <stdio.h>
#include <math.h>

/* Private define ------------------------------------------------------------*/
#define MAX_SAMPLES    32
#define ADC_MAX        4095UL

#define SCREEN_METER   0
#define SCREEN_BAR     1
#define SCREEN_OSC     2
#define SCREEN_MENU    3

#define TOTAL_SCREENS  3

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

static uint16_t adc_value = 0;

static uint16_t waveform_buffer[MAX_SAMPLES];
static uint8_t waveform_index = 0;

static uint8_t current_screen = 0;

static int16_t last_encoder = 0;

static bool menu_mode = false;
static uint8_t menu_selected = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);

/* USER CODE BEGIN PFP */

static void draw_analog_meter(uint16_t value);
static void draw_bar_graph(long long value);
static void draw_waveform(void);

static void add_to_waveform(uint16_t value);

static void draw_menu(void);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

static void draw_menu(void)
{
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("=== MENU ===", Font_7x10, White);

    ssd1306_SetCursor(0, 18);
    if (menu_selected == 0)
    
        ssd1306_WriteString(" >", Font_7x10, White);
    else
        ssd1306_WriteString("  ", Font_7x10, White);
    
    ssd1306_WriteString(" Meter", Font_7x10, White);

    
    ssd1306_SetCursor(0, 36);
    if (menu_selected == 1)
    
        ssd1306_WriteString(" >", Font_7x10, White);
    else
        ssd1306_WriteString("  ", Font_7x10, White);

    
    ssd1306_WriteString(" Bar", Font_7x10, White);

    
    ssd1306_SetCursor(0, 56); 
    if (menu_selected == 2)
    
        ssd1306_WriteString(" >", Font_7x10, White);
    else
        ssd1306_WriteString("  ", Font_7x10, White);

    ssd1306_SetCursor(0, 74);   
    ssd1306_WriteString(" Oscilloscope", Font_7x10, White);

    
    ssd1306_WriteString("Hold BTN + rotate", Font_7x10, White);
}

/**
  * @brief  Круглый стрелочный прибор
  */
static void draw_analog_meter(uint16_t value)
{
    const uint8_t cx = 64;
    const uint8_t cy = 40;
    const uint8_t r  = 30;

    const float start_deg = 0;
    const float end_deg   = 180.0f;

    float angle =
        start_deg +
        (end_deg - start_deg) * (float)value / ADC_MAX;

    ssd1306_DrawArc(
        cx,
        cy,
        r,
        (uint16_t)(start_deg + 90),
        180,
        White
    );

    for (int p = 0; p <= 100; p += 25)
    {
        float a =
            start_deg +
            (end_deg - start_deg) * p / 100.0f;

        float rad = a * M_PI / 180.0f;

        int x1 = cx + (int)((r - 3) * cosf(rad));
        int y1 = cy + (int)((r - 3) * sinf(rad));

        int x2 = cx + (int)(r * cosf(rad));
        int y2 = cy + (int)(r * sinf(rad));

        ssd1306_Line(x1, y1, x2, y2, White);
    }

    float rad_needle = angle * M_PI / 180.0f;

    int16_t nx =
        cx + (int)((r - 5) * cosf(rad_needle));

    int16_t ny =
        cy + (int)((r - 5) * sinf(rad_needle));

    ssd1306_Line(cx, cy, nx, ny, White);

    ssd1306_FillCircle(cx, cy, 3, White);

    char buf[16];

    sprintf(buf, "%4u", value);

    ssd1306_SetCursor(54, 55);
    ssd1306_WriteString(buf, Font_7x10, White);

    ssd1306_SetCursor(94, 55);
    ssd1306_WriteString("ADC", Font_7x10, White);
}

/**
  * @brief  Горизонтальная гистограмма
  */
static void draw_bar_graph(long long value)
{
    uint8_t percent =
        (value * 100) / ADC_MAX;

    uint8_t bar_len =
        (percent * 108) / 100;

    ssd1306_DrawRectangle(
        10,
        20,
        118,
        38,
        White
    );

    ssd1306_FillRectangle(
        11,
        21,
        10 + bar_len,
        37,
        White
    );

    char buf[20];

    sprintf(buf, "ADC: %ld", value);

    ssd1306_SetCursor(10, 5);
    ssd1306_WriteString(buf, Font_7x10, White);

    sprintf(buf, "%d %%", percent);

    ssd1306_SetCursor(10, 48);
    ssd1306_WriteString(buf, Font_7x10, White);
}

/**
  * @brief Добавление точки в буфер
  */
static void add_to_waveform(uint16_t value)
{
    if (waveform_index < MAX_SAMPLES)
    {
        waveform_buffer[waveform_index++] = value;
    }
    else
    {
        for (uint16_t i = 1; i < MAX_SAMPLES; i++)
        {
            waveform_buffer[i - 1] =
                waveform_buffer[i];
        }

        waveform_buffer[MAX_SAMPLES - 1] =
            value;
    }
}

/**
  * @brief Осциллограф
  */
static void draw_waveform(void)
{
    uint16_t n_points =
        (waveform_index < MAX_SAMPLES)
            ? waveform_index
            : MAX_SAMPLES;

    if (n_points < 2)
        return;

    int prev_x = 0;

    int prev_y =
        60 -
        (waveform_buffer[0] * 56) / ADC_MAX;

    for (uint16_t i = 1; i < n_points; i++)
    {
        int x =
            i * 127 / (MAX_SAMPLES - 1);

        int y =
            60 -
            (waveform_buffer[i] * 56) / ADC_MAX;

        ssd1306_Line(
            prev_x,
            prev_y,
            x,
            y,
            White
        );

        prev_x = x;
        prev_y = y;
    }

    ssd1306_SetCursor(0, 0);

    char title[16];

    sprintf(title, "OSC: %d", adc_value);

    ssd1306_WriteString(
        title,
        Font_7x10,
        White
    );
}

/* USER CODE END 0 */

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_TIM3_Init();

    HAL_Delay(1000);

    ssd1306_Init();

    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    HAL_TIM_Encoder_Start(
        &htim3,
        TIM_CHANNEL_ALL
    );

    __HAL_TIM_SET_COUNTER(&htim3, 0);

    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        waveform_buffer[i] = 0;
    }

    while (1)
    {
        // =========================
        // ADC
        // =========================

        HAL_ADC_Start(&hadc1);

        HAL_ADC_PollForConversion(
            &hadc1,
            10
        );

        adc_value =
            HAL_ADC_GetValue(&hadc1);

        HAL_ADC_Stop(&hadc1);

        add_to_waveform(adc_value);

        // =========================
        // Encoder
        // =========================

        int32_t encoder_now =
            __HAL_TIM_GET_COUNTER(&htim3);

        // Кнопка нажата?
        bool button_pressed =
            (HAL_GPIO_ReadPin(
                 GPIOA,
                 GPIO_PIN_5
             ) == GPIO_PIN_RESET);

        if (button_pressed)
        {
            menu_mode = true;
        }
        else
        {
            if (menu_mode)
            {
                current_screen = menu_selected;
            }

            menu_mode = false;
        }

        // вращение энкодера
        if (encoder_now != last_encoder)
        {
            if (menu_mode)
            {
                if (encoder_now > last_encoder)
                {
                    if (menu_selected < (TOTAL_SCREENS - 1))
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

        // =========================
        // DRAW
        // =========================

        ssd1306_Fill(Black);

        if (menu_mode)
        {
            draw_menu();
        }
        else
        {
            switch (current_screen)
            {
                case SCREEN_METER:

                    ssd1306_SetCursor(0, 0);
                    ssd1306_WriteString(
                        "METER",
                        Font_7x10,
                        White
                    );

                    draw_analog_meter(encoder_now*150);

                    break;

                case SCREEN_BAR:

                    ssd1306_SetCursor(0, 0);
                    ssd1306_WriteString(
                        "BAR",
                        Font_7x10,
                        White
                    );

                    draw_bar_graph(encoder_now*50);

                    break;

                case SCREEN_OSC:

                    draw_waveform();

                    break;
            }
        }

        ssd1306_UpdateScreen();

        HAL_Delay(20);
    }
}

/* ========================= */
/* CLOCK */
/* ========================= */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ControlVoltageScaling(
        PWR_REGULATOR_VOLTAGE_SCALE1
    );

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;

    RCC_OscInitStruct.HSIDiv =
        RCC_HSI_DIV1;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_ON;

    RCC_OscInitStruct.PLL.PLLSource =
        RCC_PLLSOURCE_HSI;

    RCC_OscInitStruct.PLL.PLLM =
        RCC_PLLM_DIV1;

    RCC_OscInitStruct.PLL.PLLN = 8;

    RCC_OscInitStruct.PLL.PLLP =
        RCC_PLLP_DIV2;

    RCC_OscInitStruct.PLL.PLLR =
        RCC_PLLR_DIV2;

    if (HAL_RCC_OscConfig(
            &RCC_OscInitStruct
        ) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_2
        ) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ========================= */
/* ADC */
/* ========================= */

static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;

    hadc1.Init.ClockPrescaler =
        ADC_CLOCK_SYNC_PCLK_DIV2;

    hadc1.Init.Resolution =
        ADC_RESOLUTION_12B;

    hadc1.Init.DataAlign =
        ADC_DATAALIGN_RIGHT;

    hadc1.Init.ScanConvMode =
        ADC_SCAN_DISABLE;

    hadc1.Init.EOCSelection =
        ADC_EOC_SINGLE_CONV;

    hadc1.Init.LowPowerAutoWait =
        DISABLE;

    hadc1.Init.LowPowerAutoPowerOff =
        DISABLE;

    hadc1.Init.ContinuousConvMode =
        DISABLE;

    hadc1.Init.NbrOfConversion = 1;

    hadc1.Init.DiscontinuousConvMode =
        DISABLE;

    hadc1.Init.ExternalTrigConv =
        ADC_SOFTWARE_START;

    hadc1.Init.ExternalTrigConvEdge =
        ADC_EXTERNALTRIGCONVEDGE_NONE;

    hadc1.Init.DMAContinuousRequests =
        DISABLE;

    hadc1.Init.Overrun =
        ADC_OVR_DATA_PRESERVED;

    hadc1.Init.SamplingTimeCommon1 =
        ADC_SAMPLETIME_1CYCLE_5;

    hadc1.Init.SamplingTimeCommon2 =
        ADC_SAMPLETIME_1CYCLE_5;

    hadc1.Init.OversamplingMode =
        DISABLE;

    hadc1.Init.TriggerFrequencyMode =
        ADC_TRIGGER_FREQ_HIGH;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_3;

    sConfig.Rank =
        ADC_REGULAR_RANK_1;

    sConfig.SamplingTime =
        ADC_SAMPLINGTIME_COMMON_1;

    if (HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK)
    {
        Error_Handler();
    }
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

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }

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
/* TIM3 ENCODER */
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

    sConfig.IC1Filter = 8;

    sConfig.IC2Polarity =
        TIM_ICPOLARITY_RISING;

    sConfig.IC2Selection =
        TIM_ICSELECTION_DIRECTTI;

    sConfig.IC2Prescaler =
        TIM_ICPSC_DIV1;

    sConfig.IC2Filter = 8;

    if (HAL_TIM_Encoder_Init(
            &htim3,
            &sConfig
        ) != HAL_OK)
    {
        Error_Handler();
    }

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

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // BUTTON

    GPIO_InitStruct.Pin = GPIO_PIN_5;

    GPIO_InitStruct.Mode =
        GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull =
        GPIO_PULLUP;

    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );

    // ADC

    GPIO_InitStruct.Pin = GPIO_PIN_3;

    GPIO_InitStruct.Mode =
        GPIO_MODE_ANALOG;

    GPIO_InitStruct.Pull =
        GPIO_NOPULL;

    HAL_GPIO_Init(
        GPIOB,
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

#ifdef USE_FULL_ASSERT

void assert_failed(
    uint8_t *file,
    uint32_t line
)
{
}

#endif
