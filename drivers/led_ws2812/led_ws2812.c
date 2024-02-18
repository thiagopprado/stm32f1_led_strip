/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @brief Infrared module implementation.
 * 
 */
/* Includes ------------------------------------------------------------------*/
#include "led_ws2812.h"

#include <stdbool.h>
#include <stddef.h>

#include "stm32f1xx.h"
#include "core_cm3.h"

#include "stm32f1xx_hal.h"

/* Private types -------------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define BIT_CHECK(value, bit)       ((value >> bit) & 0x01)
#define BIT_SET(value, bit)         (value |= 1 << bit)
#define BIT_CLEAR(value, bit)       (value &= ~(1 << bit))

#define LED_WS2812_DUTY_CYCLE_BIT_0     30
#define LED_WS2812_DUTY_CYCLE_BIT_1     50
#define LED_WS2812_BITS_NR              24

/**
 * @brief PWM prescaler.
 * 
 * This generates a 800khz frequency.
 * @{
 */
#define LED_WS2812_PSC  0
#define LED_WS2812_ARR  89
/** @} */

/**
 * @brief PWM buffer size.
 * 
 * Each bit represents a PWM duty cycle value. PWM buffer has the same positions number
 * as the number of bits needed to update all LEDs plus one. The final duty cycle is always
 * zero, to stop sending new bits.
 */
#define LED_WS2812_PWM_BUFFER_SZ    ((LED_WS2812_BITS_NR * LED_WS2812_NR) + 1)    

/* Private variables ---------------------------------------------------------*/
static TIM_HandleTypeDef timer_handle = { 0 };
static DMA_HandleTypeDef dma_handle = { 0 };

static uint16_t led_pwm_buffer[LED_WS2812_PWM_BUFFER_SZ] = { 0 };
static volatile bool trx_in_progress = false;

/* Private function prototypes -----------------------------------------------*/
static void pwm_pulse_finished_callback(TIM_HandleTypeDef *htim);

/* Private function implementation--------------------------------------------*/
/**
 * @brief DMA transmission complete callback.
 * 
 * @param htim  Timer handle.
 */
static void pwm_pulse_finished_callback(TIM_HandleTypeDef *htim)
{
    HAL_TIM_PWM_Stop_DMA(htim, LED_WS2812_PWM_CH);
    trx_in_progress = false;
}

/* Public functions ----------------------------------------------------------*/
/**
 * @brief Sets up LED ws2812 driver.
 * 
 * The communication is done varying the PWM duty cycle at a fixed frequency.
 * The duty cycle is updated via DMA.
 */
void led_ws2812_setup(void) {
    GPIO_InitTypeDef gpio_init = { 0 };

    // GPIO
    LED_WS2812_GPIO_EN();
    gpio_init.Pin = LED_WS2812_PIN;
    gpio_init.Mode = GPIO_MODE_AF_OD;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED_WS2812_PORT, &gpio_init);

    HAL_NVIC_SetPriority(LED_WS2812_DMA_IRQN, LED_WS2812_DMA_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(LED_WS2812_DMA_IRQN);

    // DMA
    LED_WS2812_DMA_EN();
    dma_handle.Instance = LED_WS2812_DMA_INSTANCE;
    dma_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle.Init.MemInc = DMA_MINC_ENABLE;
    dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    dma_handle.Init.Mode = DMA_NORMAL;
    dma_handle.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    HAL_DMA_Init(&dma_handle);

    dma_handle.Parent = &timer_handle;

    // PWM Timer
    LED_WS2812_TIMER_EN();
    timer_handle.Instance = LED_WS2812_TIMER;
    timer_handle.Init.Prescaler = LED_WS2812_PSC;
    timer_handle.Init.Period = LED_WS2812_ARR;
    timer_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    timer_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timer_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    timer_handle.hdma[TIM_DMA_ID_CC4] = &dma_handle;
    HAL_TIM_PWM_Init(&timer_handle);
    HAL_TIM_RegisterCallback(&timer_handle, HAL_TIM_PWM_PULSE_FINISHED_CB_ID, pwm_pulse_finished_callback);

    TIM_OC_InitTypeDef pwm_config = { 0 };
    pwm_config.OCMode = TIM_OCMODE_PWM1;
    HAL_TIM_PWM_ConfigChannel(&timer_handle, &pwm_config, LED_WS2812_PWM_CH);
}

/**
 * @brief Updates the LEDs.
 * 
 * @param color     Color buffer.
 * @param led_nr    Number of LEDs to be updated.
 */
void led_ws2812_write(uint32_t *color, uint16_t led_nr) {
    uint32_t pwm_idx = 0;

    while (trx_in_progress == true);

    for (uint16_t led_idx = 0; led_idx < led_nr; led_idx++) {
        for (int32_t i = (LED_WS2812_BITS_NR - 1); i >= 0; i--) {
            if (BIT_CHECK(color[led_idx], i) == 0) {
                led_pwm_buffer[pwm_idx++] = LED_WS2812_DUTY_CYCLE_BIT_0;
            } else {
                led_pwm_buffer[pwm_idx++] = LED_WS2812_DUTY_CYCLE_BIT_1;
            }
        }
    }

    led_pwm_buffer[pwm_idx++] = 0;

    // Init transmission
    trx_in_progress = true;
    HAL_TIM_PWM_Start_DMA(&timer_handle, LED_WS2812_PWM_CH, (uint32_t*)led_pwm_buffer, pwm_idx);
}

/**
 * @brief PWM DMA ISR.
 */
void DMA1_Channel3_IRQHandler(void) {
    HAL_DMA_IRQHandler(&dma_handle);
}
