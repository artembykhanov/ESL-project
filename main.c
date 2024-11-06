#include <stdbool.h>
#include <stdint.h>
#include "boards.h"
#include "nrf_gpio.h"
#include "nrfx_systick.h"
#include "nrfx_gpiote.h"

#define DEVICE_ID 6585
#define PWM_FREQ 1000                  // Частота ШИМ: 1 кГц
#define PERIOD_US (1000000 / PWM_FREQ) // Период 1 мс для 1 кГц
#define MAX_DUTY_CYCLE 100             // Время для дебаунса
#define DELAY_AFTER_BLINK_MS 1000

#define LED_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_B_PIN NRF_GPIO_PIN_MAP(0, 12)

#define BUTTON_PIN NRF_GPIO_PIN_MAP(1, 6)

#define DIGIT1 (DEVICE_ID / 1000)
#define DIGIT2 ((DEVICE_ID / 100) % 10)
#define DIGIT3 ((DEVICE_ID / 10) % 10)
#define DIGIT4 (DEVICE_ID % 10)

volatile bool flag = false;

int index = 0;
int len_led_arr;

void init_pin();
void init_led_arr(int *led_arr);
void pwm_led_fade(int pin);
void turn_off_all_leds(void);
void BUTTON_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);
    init_pin();
    nrfx_systick_init();

    len_led_arr = DIGIT1 + DIGIT2 + DIGIT3 + DIGIT4;
    int led_arr[len_led_arr];
    init_led_arr(led_arr);

    turn_off_all_leds();
    while (true)
    {
        while (flag)
        {
            pwm_led_fade(led_arr[index]);
            nrfx_systick_delay_ms(DELAY_AFTER_BLINK_MS);
            index = (index + 1) % len_led_arr;
        }
    }
}

void pwm_led_fade(int pin)
{
    nrfx_systick_state_t start_time;

    for (int duty_cycle = 0; duty_cycle <= MAX_DUTY_CYCLE; duty_cycle++)
    {
        uint32_t on_time = (duty_cycle * PERIOD_US) / 100;
        uint32_t off_time = PERIOD_US - on_time;

        nrf_gpio_pin_write(pin, 0);
        nrfx_systick_get(&start_time);
        while (!nrfx_systick_test(&start_time, on_time))
        {
        }

        nrf_gpio_pin_write(pin, 1);
        nrfx_systick_get(&start_time);
        while (!nrfx_systick_test(&start_time, off_time))
        {
        }
    }

    for (int duty_cycle = MAX_DUTY_CYCLE; duty_cycle >= 0; duty_cycle--)
    {
        uint32_t on_time = (duty_cycle * PERIOD_US) / 100;
        uint32_t off_time = PERIOD_US - on_time;

        nrf_gpio_pin_write(pin, 0);
        nrfx_systick_get(&start_time);
        while (!nrfx_systick_test(&start_time, on_time))
        {
        }

        nrf_gpio_pin_write(pin, 1);
        nrfx_systick_get(&start_time);
        while (!nrfx_systick_test(&start_time, off_time))
        {
        }
    }
}

void init_pin(void)
{
    nrf_gpio_cfg_output(LED_PIN);
    nrf_gpio_cfg_output(LED_R_PIN);
    nrf_gpio_cfg_output(LED_G_PIN);
    nrf_gpio_cfg_output(LED_B_PIN);

    nrfx_gpiote_init();
    nrfx_gpiote_in_config_t button_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    button_config.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BUTTON_PIN, &button_config, BUTTON_IRQHandler);
    nrfx_gpiote_in_event_enable(BUTTON_PIN, true);
}

void BUTTON_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    flag = !flag;
}

void init_led_arr(int *led_arr)
{
    int index = 0;

    for (int i = 0; i < DIGIT1; i++)
    {
        led_arr[index++] = LED_PIN;
    }
    for (int i = 0; i < DIGIT2; i++)
    {
        led_arr[index++] = LED_R_PIN;
    }
    for (int i = 0; i < DIGIT3; i++)
    {
        led_arr[index++] = LED_G_PIN;
    }
    for (int i = 0; i < DIGIT4; i++)
    {
        led_arr[index++] = LED_B_PIN;
    }
}

void turn_off_all_leds(void)
{
    nrf_gpio_pin_write(LED_PIN, 1);
    nrf_gpio_pin_write(LED_R_PIN, 1);
    nrf_gpio_pin_write(LED_G_PIN, 1);
    nrf_gpio_pin_write(LED_B_PIN, 1);
}
