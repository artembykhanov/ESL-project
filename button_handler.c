#include "button_handler.h"

#include "nrf_gpio.h"
#include "nrfx_gpiote.h"

#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

APP_TIMER_DEF(timer_debounce);
APP_TIMER_DEF(timer_double_click);
APP_TIMER_DEF(timer_long_press);
APP_TIMER_DEF(timer_long_press_repeat);

static volatile bool first_click_detected = false;
static volatile bool double_click_detected = false;
static volatile bool long_press_active = false;

static volatile uint8_t click_counter = 0;

static click_callback callback_single_click = NULL;
static click_callback callback_double_click = NULL;
static click_callback callback_long_press = NULL;

void button_event_init(click_callback on_single_click, click_callback on_double_click, click_callback on_long_press)
{
    callback_single_click = on_single_click;
    callback_double_click = on_double_click;
    callback_long_press = on_long_press;
}

void button_pin_init(void)
{
    nrfx_gpiote_init();

    nrfx_gpiote_in_config_t button_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    button_config.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BUTTON_PIN, &button_config, button_interrupt_handler);
    nrfx_gpiote_in_event_enable(BUTTON_PIN, true);
}

void button_timers_init(void)
{
    app_timer_init();
    app_timer_create(&timer_debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_timer_handler);
    app_timer_create(&timer_double_click, APP_TIMER_MODE_SINGLE_SHOT, double_click_timer_handler);
    app_timer_create(&timer_long_press, APP_TIMER_MODE_SINGLE_SHOT, long_press_timer_handler);
    app_timer_create(&timer_long_press_repeat, APP_TIMER_MODE_REPEATED, long_press_repeat_timer_handler);
}

void button_interrupt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (is_button_pressed())
    {
        app_timer_start(timer_debounce, DEBOUNCE_INTERVAL, NULL);
    }
}

void debounce_timer_handler(void *p_context)
{
    if (is_button_pressed())
    {
        if (!long_press_active)
        {
            app_timer_start(timer_long_press, LONG_PRESS_INITIAL_INTERVAL, NULL);
        }

        process_button_click();
    }
    else
    {
        app_timer_stop(timer_long_press);
        app_timer_stop(timer_long_press_repeat);
        long_press_active = false;
    }
}

void process_button_click(void)
{
    click_counter++;

    app_timer_start(timer_double_click, DOUBLE_CLICK_INTERVAL, NULL);

    switch (click_counter)
    {
    case 1:
        if (callback_single_click)
        {
            callback_single_click();
        }
        break;
    case 2:
        if (callback_double_click)
        {
            callback_double_click();
        }
        break;

    default:
        break;
    }
}

void double_click_timer_handler(void *p_context)
{
    app_timer_stop(timer_double_click);
    first_click_detected = false;
    click_counter = 0;
}

void long_press_timer_handler(void *p_context)
{
    if (is_button_pressed())
    {
        long_press_active = true;

        if (callback_long_press)
        {
            callback_long_press();
        }

        app_timer_start(timer_long_press_repeat, LONG_PRESS_REPEAT_INTERVAL, NULL);
    }
}

void long_press_repeat_timer_handler(void *p_context)
{
    if (is_button_pressed())
    {
        if (callback_long_press)
        {
            callback_long_press();
        }
    }
    else
    {
        app_timer_stop(timer_long_press_repeat);
        long_press_active = false;
    }
}

bool is_button_pressed(void)
{
    return nrf_gpio_pin_read(BUTTON_PIN) == 0;
}
