/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @brief Infrared module.
 * 
 */
#ifndef LED_WS2812_H
#define LED_WS2812_H

#include <stdint.h>

/**
 * @brief LED pins.
 * 
 * @note Changing these values require changing the DMA channel beeing used.
 * @{
 */
#define LED_WS2812_GPIO_EN()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED_WS2812_PORT         GPIOB
#define LED_WS2812_PIN          GPIO_PIN_1
#define LED_WS2812_TIMER        TIMER_3
#define LED_WS2812_PWM_CH       TIMER_CH_4
/** @} */

#if !defined (LED_WS2812_NR)
    #define LED_WS2812_NR       30
#endif

#define LED_WS2812_COLOR_MAX    255

#define LED_WS2812_GET_R(value)    (((value) & 0x0000FF) << 8)
#define LED_WS2812_GET_G(value)    (((value) & 0x0000FF) << 16)
#define LED_WS2812_GET_B(value)    ((value) & 0x0000FF)

void led_ws2812_setup(void);
void led_ws2812_write(uint32_t *color, uint16_t led_nr);

#endif /* LED_WS2812_H */