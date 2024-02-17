/** Includes ------------------------------------------------------ */
#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "core_cm3.h"

#include "buzzer.h"

#include "led_strip.h"

#include "stm32f1xx_hal.h"

/** Definitions --------------------------------------------------- */
/**
 * @brief Update time.
 * @{
 */
#define LED_UPDATE_TIME_MS      5
#define BUZZER_NOTE_TIME_MS     100
/** @} */

/** Types --------------------------------------------------------- */

/** Variables ----------------------------------------------------- */
static buzzer_note_t sheet_music[] = {
    BUZZER_NOTE_E4, BUZZER_NOTE_ST, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_ST, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_ST, BUZZER_NOTE_G4, BUZZER_NOTE_ST,
    BUZZER_NOTE_C4, BUZZER_NOTE_ST, BUZZER_NOTE_D4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4,
    BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4,
    BUZZER_NOTE_F4, BUZZER_NOTE_ST, BUZZER_NOTE_F4, BUZZER_NOTE_ST,
    BUZZER_NOTE_F4, BUZZER_NOTE_F4, BUZZER_NOTE_F4, BUZZER_NOTE_ST,
    BUZZER_NOTE_F4, BUZZER_NOTE_ST, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_ST, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_G4, BUZZER_NOTE_ST, BUZZER_NOTE_G4, BUZZER_NOTE_ST,
    BUZZER_NOTE_F4, BUZZER_NOTE_ST, BUZZER_NOTE_D4, BUZZER_NOTE_ST,
    BUZZER_NOTE_C4, BUZZER_NOTE_C4, BUZZER_NOTE_C4, BUZZER_NOTE_C4,
    BUZZER_NOTE_C4, BUZZER_NOTE_C4, BUZZER_NOTE_C4, BUZZER_NOTE_C4,
    BUZZER_NOTE_ST, BUZZER_NOTE_ST, BUZZER_NOTE_ST, BUZZER_NOTE_ST,
    BUZZER_NOTE_ST, BUZZER_NOTE_ST, BUZZER_NOTE_ST, BUZZER_NOTE_ST,
};

/** Prototypes ---------------------------------------------------- */
static void clock_config(void);

/** Internal functions -------------------------------------------- */
/**
 * @brief MCU clock configuration.
 */
static void clock_config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/** Public functions ---------------------------------------------- */
int main(void) {
    uint32_t led_update_timeshot = 0;
    uint32_t buzzer_timeshot = 0;
    uint32_t note_idx = 0;

    HAL_Init();
    clock_config();

    buzzer_setup();
    led_setup();

    while (true) {
        if (HAL_GetTick() - led_update_timeshot >= LED_UPDATE_TIME_MS) {
            led_update_timeshot = HAL_GetTick();

            led_update();
        }

        if (HAL_GetTick() - buzzer_timeshot >= BUZZER_NOTE_TIME_MS) {
            buzzer_timeshot = HAL_GetTick();

            buzzer_play_note(sheet_music[note_idx]);

            note_idx++;
            if (note_idx >= (sizeof(sheet_music) / sizeof(sheet_music[0]))) {
                note_idx = 0;
            }
        }
    }
}
