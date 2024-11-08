#include <stdbool.h>
#include <stdint.h>
#include "boards.h"
#include "nrf_gpio.h"
#include "nrfx_systick.h"
#include "nrfx_gpiote.h"
#include "app_timer.h"
#include "drv_rtc.h"
#include "nrf_drv_clock.h"

#include "nordic_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#define DEVICE_ID 6585
#define PWM_FREQ 1000
#define PERIOD_US (1000000 / PWM_FREQ)
#define DUTY_CYCLE_MAX 100
#define DUTY_CYCLE_STEP 0.1
#define DELAY_AFTER_BLINK_MS 1000

#define LED_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_B_PIN NRF_GPIO_PIN_MAP(0, 12)

#define BUTTON_PIN NRF_GPIO_PIN_MAP(1, 6)

#define DOUBLE_CLICK_INTERVAL APP_TIMER_TICKS(250)
#define DEBOUNCE_INTERVAL APP_TIMER_TICKS(50)

#define DIGIT1 (DEVICE_ID / 1000)
#define DIGIT2 ((DEVICE_ID / 100) % 10)
#define DIGIT3 ((DEVICE_ID / 10) % 10)
#define DIGIT4 (DEVICE_ID % 10)

int index = 0;
int len_led_arr;
volatile bool is_first_click_done;
volatile bool double_click;
volatile bool freeze_sequence = true;

APP_TIMER_DEF(debounce_timer);
APP_TIMER_DEF(double_click_timer);

void init_pin(void);
void init_timer(void);
void init_led_arr(int *led_arr);
void pwm_led_fade(int pin);
void pwm_cycle(int pin, double duty_cycle);
void turn_on_led(int pin);
void turn_off_led(int pin);
void turn_off_all_leds(void);
void BUTTON_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void debounce_timeout_handler(void *p_context);
void double_click_timeout_handler(void *p_context);
void init_logs(void);
void lfclk_request(void);

bool is_button_press(void);

int main(void)
{
    init_logs();
    NRF_LOG_INFO("Starting the main application");
    lfclk_request();

    bsp_board_init(BSP_INIT_LEDS);
    init_pin();
    init_timer();
    nrfx_systick_init();

    len_led_arr = DIGIT1 + DIGIT2 + DIGIT3 + DIGIT4;
    int led_arr[len_led_arr];
    init_led_arr(led_arr);

    turn_off_all_leds();

    while (true)
    {
        if (double_click)
        {
            nrfx_systick_delay_ms(DELAY_AFTER_BLINK_MS / 2);
            pwm_led_fade(led_arr[index]);
            nrfx_systick_delay_ms(DELAY_AFTER_BLINK_MS / 2);

            index = (index + 1) % len_led_arr;
        }
    }
}

void pwm_led_fade(int pin)
{
    for (double duty_cycle = 0; duty_cycle <= DUTY_CYCLE_MAX; duty_cycle += DUTY_CYCLE_STEP)
    {
        while (true)
        {
            if (freeze_sequence)
            {
                pwm_cycle(pin, duty_cycle);
            }
            else
            {
                pwm_cycle(pin, duty_cycle);
                break;
            }
        }
    }

    for (double duty_cycle = DUTY_CYCLE_MAX; duty_cycle >= 0; duty_cycle -= DUTY_CYCLE_STEP)
    {
        while (true)
        {
            if (freeze_sequence)
            {
                pwm_cycle(pin, duty_cycle);
            }
            else
            {
                pwm_cycle(pin, duty_cycle);
                break;
            }
        }
    }
}

void pwm_cycle(int pin, double duty_cycle)
{
    nrfx_systick_state_t start_time;
    uint32_t on_time = (duty_cycle * PERIOD_US) / DUTY_CYCLE_MAX;
    uint32_t off_time = PERIOD_US - on_time;

    turn_on_led(pin);
    nrfx_systick_get(&start_time);
    while (!nrfx_systick_test(&start_time, on_time))
    {
    }

    turn_off_led(pin);
    nrfx_systick_get(&start_time);
    while (!nrfx_systick_test(&start_time, off_time))
    {
    }
}

void button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (is_button_press())
    {
        NRF_LOG_INFO("Button press detected");
        app_timer_start(debounce_timer, DEBOUNCE_INTERVAL, NULL);
    }
}

void debounce_timeout_handler(void *p_context)
{
    if (is_button_press())
    {
        if (is_first_click_done)
        {
            app_timer_stop(double_click_timer);
            is_first_click_done = false;
            double_click = !double_click;
            freeze_sequence = !freeze_sequence;
            NRF_LOG_INFO("Double click detected");
        }
        else
        {
            is_first_click_done = true;
            NRF_LOG_INFO("Single click detected, starting double-click timer");
            app_timer_start(double_click_timer, DOUBLE_CLICK_INTERVAL, NULL);
        }
    }
}

void double_click_timeout_handler(void *p_context)
{
    is_first_click_done = false;
    NRF_LOG_INFO("Double-click timer expired, resetting first click status");
}

void turn_on_led(int pin)
{
    nrf_gpio_pin_write(pin, 0);
    NRF_LOG_DEBUG("Turning on LED at pin %d", pin);
}
void turn_off_led(int pin)
{
    nrf_gpio_pin_write(pin, 1);
    NRF_LOG_DEBUG("Turning off LED at pin %d", pin);
}

void turn_off_all_leds(void)
{
    turn_off_led(LED_PIN);
    turn_off_led(LED_R_PIN);
    turn_off_led(LED_G_PIN);
    turn_off_led(LED_B_PIN);
}

bool is_button_press(void)
{
    return nrf_gpio_pin_read(BUTTON_PIN) == 0;
}

/**@brief Function starting the internal LFCLK oscillator.
 *
 * @details This is needed by RTC1 which is used by the Application Timer
 *          (When SoftDevice is enabled the LFCLK is always running and this is not needed).
 */
void lfclk_request(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}

void init_logs(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("Log system initialized.");
}

void init_pin(void)
{
    nrf_gpio_cfg_output(LED_PIN);
    nrf_gpio_cfg_output(LED_R_PIN);
    nrf_gpio_cfg_output(LED_G_PIN);
    nrf_gpio_cfg_output(LED_G_PIN);

    nrfx_gpiote_init();
    nrfx_gpiote_in_config_t button_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    button_config.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BUTTON_PIN, &button_config, button_handler);
    nrfx_gpiote_in_event_enable(BUTTON_PIN, true);
}

void init_timer(void)
{
    app_timer_init();
    app_timer_create(&debounce_timer, APP_TIMER_MODE_SINGLE_SHOT, debounce_timeout_handler);
    app_timer_create(&double_click_timer, APP_TIMER_MODE_SINGLE_SHOT, double_click_timeout_handler);
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