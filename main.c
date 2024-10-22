/** @file
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

void led_blink(int i);

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    /* Configure board. */
    bsp_board_init(BSP_INIT_LEDS);

    // My device id #6585
    int device_id = 6585;

    int digit1 = device_id / 1000;
    int digit2 = (device_id / 100) % 10;
    int digit3 = (device_id / 10) % 10;
    int digit4 = device_id % 10;

    /* Toggle LEDs. */
    while (true)
    {

        for (int i = 0; i < digit1; i++)
        {
            led_blink(0);
        }
        nrf_delay_ms(2000);

        for (int i = 0; i < digit2; i++)
        {
            led_blink(1);
        }
        nrf_delay_ms(2000);

        for (int i = 0; i < digit3; i++)
        {
            led_blink(2);
        }
        nrf_delay_ms(2000);

        for (int i = 0; i < digit4; i++)
        {
            led_blink(3);
        }

        // after displaying the ID, turn on all the LEDs, this means the end of the program
        nrf_delay_ms(3000);
        bsp_board_leds_on();
        nrf_delay_ms(2000);
        bsp_board_leds_off();
        nrf_delay_ms(3000);
    }
}

void led_blink(int i)
{
    bsp_board_led_invert(i);
    nrf_delay_ms(400);
    bsp_board_led_invert(i);
    nrf_delay_ms(400);
}
/**
 *@}
 **/
