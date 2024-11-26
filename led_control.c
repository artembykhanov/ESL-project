#include "led_control.h"
#include "pwm_control.h"

#include <math.h>

#include "nrfx_pwm.h"
#include "nrf_gpio.h"

#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"

static bool increasing_brightness = false;
static bool increasing_saturation = false;

static bool increasing_hue_LED1 = true;
static bool increasing_saturation_LED1 = true;

static uint16_t value_duty_LED1 = 0;

static void hsv_to_rgb_float();
static void change_value_smoothly(uint16_t *value, bool *increasing, uint16_t min_value, uint16_t max_value, uint16_t step);

controller_mode current_mode = MODE_AFK;

const char *controller_mode_strings[] = {
    "MODE_AFK",
    "MODE_HUE",
    "MODE_SATURATION",
    "MODE_BRIGHTNESS"};

mode_steps current_mode_step = {
    .afk_const = 0,
    .hue_step = 3,
    .saturation_step = 15,
    .brightness_const = PWM_TOP_VALUE};

RGB_color RGB = {
    .red = 0,
    .green = 0,
    .blue = 0};

HSB_color HSB = {
    .hue = (uint16_t)360 * 0.85,
    .saturation = PWM_TOP_VALUE,
    .brightness = PWM_TOP_VALUE};

void set_current_mode(void)
{
    current_mode = (current_mode + 1) % 4;
    NRF_LOG_INFO("Current mode: %s", controller_mode_strings[(int)current_mode]);
}

void update_duty_cycle_RGB(void)
{
    switch (current_mode)
    {
    case MODE_HUE:
        HSB.hue = (HSB.hue + 1) % 360;
        break;

    case MODE_SATURATION:
        change_value_smoothly(&HSB.saturation, &increasing_saturation, 0, PWM_TOP_VALUE, SATURATION_STEP);
        break;

    case MODE_BRIGHTNESS:
        change_value_smoothly(&HSB.brightness, &increasing_brightness, 0, PWM_TOP_VALUE, BRIGHTNESS_STEP);
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
        change_value_smoothly(&value_duty_LED1, &increasing_hue_LED1, 0, PWM_TOP_VALUE, current_mode_step.hue_step);
        break;

    case MODE_SATURATION:
        change_value_smoothly(&value_duty_LED1, &increasing_saturation_LED1, 0, PWM_TOP_VALUE, current_mode_step.saturation_step);
        break;

    case MODE_BRIGHTNESS:
        value_duty_LED1 = current_mode_step.brightness_const;
        break;

    case MODE_AFK:
        value_duty_LED1 = current_mode_step.afk_const;
        increasing_hue_LED1 = true;
        increasing_saturation_LED1 = true;
        break;

    default:
        break;
    }
}

static void change_value_smoothly(uint16_t *value, bool *increasing, uint16_t min_value, uint16_t max_value, uint16_t step)
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
static void hsv_to_rgb_float()
{
    float hue = HSB.hue % 360;
    float saturation = HSB.saturation / 255.0f;
    float value = HSB.brightness / 255.0f;

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

    RGB.red = (uint8_t)((r_prime + m) * 255);
    RGB.green = (uint8_t)((g_prime + m) * 255);
    RGB.blue = (uint8_t)((b_prime + m) * 255);
}

void led_display_current_color(void)
{
    hsv_to_rgb_float();

    NRF_LOG_INFO("R: %d; G: %d; B: %d", RGB.red, RGB.green, RGB.blue);
    pwm_update_duty_cycle(0, value_duty_LED1);
    pwm_update_duty_cycle(1, RGB.red);
    pwm_update_duty_cycle(2, RGB.green);
    pwm_update_duty_cycle(3, RGB.blue);
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
