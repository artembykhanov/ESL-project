#include "led_control.h"
#include "pwm_control.h"

#include <math.h>

#include "nrfx_pwm.h"
#include "nrf_gpio.h"

#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static bool increasing_brightness = false;
static bool increasing_saturation = false;

static uint32_t hue = (int)360 * 0.85;
static uint32_t saturation = PWM_TOP_VALUE;
static uint32_t brightness = PWM_TOP_VALUE;

static uint32_t led_step = 0;

controller_mode_t current_mode = MODE_AFK;

steps_mode steps_for_mode = {
    .afk_mode = 0,
    .hue_mode = 3,
    .saturation_mode = 10,
    .brightness_mode = PWM_TOP_VALUE};

void set_current_mode(void)
{
    current_mode = (current_mode + 1) % 4;
}

void update_duty_cycle_RGB(void)
{
    switch (current_mode)
    {
    case MODE_HUE:
        hue = (hue + 1) % 360;
        break;

    case MODE_SATURATION:
        change_value_smoothly(&saturation, &increasing_saturation, 0, PWM_TOP_VALUE, SATURATION_STEP);
        break;

    case MODE_BRIGHTNESS:
        change_value_smoothly(&brightness, &increasing_brightness, 0, PWM_TOP_VALUE, BRIGHTNESS_STEP);
        break;

    default:
        break;
    }
}

void update_duty_cycle_LED1(void)
{
    switch (current_mode)
    {
    case MODE_HUE:
        led_step = (led_step + steps_for_mode.hue_mode) % PWM_TOP_VALUE;
        break;

    case MODE_SATURATION:
        led_step = (led_step + steps_for_mode.saturation_mode) % PWM_TOP_VALUE;
        break;

    case MODE_BRIGHTNESS:
        led_step = steps_for_mode.brightness_mode;
        break;

    case MODE_AFK:
        led_step = steps_for_mode.afk_mode;
        break;

    default:
        break;
    }
}

void change_value_smoothly(uint32_t *value, bool *increasing, uint32_t min_value, uint32_t max_value, uint32_t step)
{
    if (*increasing)
    {
        (*value) += step;
        if (*value >= max_value)
        {
            *value = max_value;
            *increasing = false;
        }
    }
    else
    {
        (*value) -= step;
        if (*value <= min_value)
        {
            *value = min_value;
            *increasing = true;
        }
    }
}

// https://www.rapidtables.org/ru/convert/color/hsv-to-rgb.html
void hsv_to_rgb_float(uint32_t h, uint32_t s, uint32_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float hue = h % 360;
    float saturation = s / 255.0f;
    float value = v / 255.0f;

    float c = value * saturation;                         // Компонента цвета
    float x = c * (1 - fabsf(fmodf(hue / 60.0f, 2) - 1)); // Промежуточное значение
    float m = value - c;                                  // Смещение для яркости

    float r_prime = 0, g_prime = 0, b_prime = 0;

    if (hue < 60)
    {
        r_prime = c;
        g_prime = x;
        b_prime = 0;
    }
    else if (hue < 120)
    {
        r_prime = x;
        g_prime = c;
        b_prime = 0;
    }
    else if (hue < 180)
    {
        r_prime = 0;
        g_prime = c;
        b_prime = x;
    }
    else if (hue < 240)
    {
        r_prime = 0;
        g_prime = x;
        b_prime = c;
    }
    else if (hue < 300)
    {
        r_prime = x;
        g_prime = 0;
        b_prime = c;
    }
    else
    {
        r_prime = c;
        g_prime = 0;
        b_prime = x;
    }

    *r = (uint8_t)((r_prime + m) * 255);
    *g = (uint8_t)((g_prime + m) * 255);
    *b = (uint8_t)((b_prime + m) * 255);
}

void led_display_current_color(void)
{
    uint8_t r, g, b;
    hsv_to_rgb_float(hue, saturation, brightness, &r, &g, &b);

    pwm_update_duty_cycle(0, led_step);
    pwm_update_duty_cycle(1, r);
    pwm_update_duty_cycle(2, g);
    pwm_update_duty_cycle(3, b);
}

void init_led_pin(void)
{
    nrf_gpio_cfg_output(LED_PIN);
    nrf_gpio_cfg_output(LED_R_PIN);
    nrf_gpio_cfg_output(LED_G_PIN);
    nrf_gpio_cfg_output(LED_B_PIN);
}

void turn_on_led(int pin)
{
    nrf_gpio_pin_write(pin, LED_TURN_ON);
}

void turn_off_led(int pin)
{
    nrf_gpio_pin_write(pin, LED_TURN_OFF);
}

void turn_off_all_leds(void)
{
    turn_off_led(LED_PIN);
    turn_off_led(LED_R_PIN);
    turn_off_led(LED_G_PIN);
    turn_off_led(LED_B_PIN);
}
