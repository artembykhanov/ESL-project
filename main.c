#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"
#include "nrf_gpio.h"

#define DEVICE_ID 6585
#define LED_ON_TIME_MS 1000
#define LED_OFF_TIME_MS 1000
#define PAUSE_AFTER_ID_MS 300

#define LED_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_B_PIN NRF_GPIO_PIN_MAP(0, 12)

#define BUTTON_PIN NRF_GPIO_PIN_MAP(1, 6)

void led_blink(int i);
void init_pin();
void init_led_arr(int *led_arr, int len_led_arr);
bool is_button_press();
void end_blink();

const int digit1 = DEVICE_ID / 1000;
const int digit2 = (DEVICE_ID / 100) % 10;
const int digit3 = (DEVICE_ID / 10) % 10;
const int digit4 = DEVICE_ID % 10;

int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);
    bsp_board_init(BSP_INIT_BUTTONS);
    init_pin();

    const int len_led_arr = digit1 + digit2 + digit3 + digit4;
    int led_arr[len_led_arr];

    init_led_arr(led_arr, len_led_arr);

    int index = 0;

    while (true)
    {
        if (is_button_press())
        {
            bsp_board_leds_off();
            nrf_delay_ms(LED_OFF_TIME_MS / 2);
            led_blink(led_arr[index++]);
            if (index >= len_led_arr)
            {
                nrf_delay_ms(LED_OFF_TIME_MS);
                end_blink();
                index = 0;
            }
        }
        else
        {
            nrf_delay_ms(100);
        }
    }
}

void led_blink(int pin)
{
    nrf_gpio_pin_write(pin, 0);

    for (int i = 0; i < LED_ON_TIME_MS / 100; i++)
    {
        if (!is_button_press())
        {
            return;
        }
        nrf_delay_ms(100);
    }

    nrf_gpio_pin_write(pin, 1);
    nrf_delay_ms(LED_OFF_TIME_MS / 2);
}

void init_led_arr(int *led_arr, int len_led_arr)
{
    int index = 0;

    for (int i = 0; i < digit1; i++)
    {
        led_arr[index++] = LED_PIN;
    }
    for (int i = 0; i < digit2; i++)
    {
        led_arr[index++] = LED_R_PIN;
    }
    for (int i = 0; i < digit3; i++)
    {
        led_arr[index++] = LED_G_PIN;
    }
    for (int i = 0; i < digit4; i++)
    {
        led_arr[index++] = LED_B_PIN;
    }
}

void init_pin()
{
    nrf_gpio_cfg_output(LED_PIN);
    nrf_gpio_cfg_output(LED_R_PIN);
    nrf_gpio_cfg_output(LED_G_PIN);
    nrf_gpio_cfg_output(LED_B_PIN);
    nrf_gpio_cfg_input(BUTTON_PIN, NRF_GPIO_PIN_PULLUP);
}

bool is_button_press()
{
    return nrf_gpio_pin_read(BUTTON_PIN) == 0;
}

void end_blink()
{
    bsp_board_leds_on();
    nrf_delay_ms(PAUSE_AFTER_ID_MS);
    bsp_board_leds_off();
    nrf_delay_ms(PAUSE_AFTER_ID_MS);
    bsp_board_leds_on();
    nrf_delay_ms(PAUSE_AFTER_ID_MS);
    bsp_board_leds_off();
    nrf_delay_ms(PAUSE_AFTER_ID_MS);
    nrf_delay_ms(LED_ON_TIME_MS);
}