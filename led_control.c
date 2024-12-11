#include "led_control.h"
#include "pwm_control.h"
#include "nvmc_control.h"

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

#define SATURATION_TOP_VALUE 100
#define BRIGHTNESS_TOP_VALUE 100

#define SATURATION_STEP 1
#define BRIGHTNESS_STEP 1

#define LED_TURN_OFF 1
#define LED_TURN_ON 0

static bool increasing_brightness = false;
static bool increasing_saturation = false;

static bool increasing_hue_LED1 = true;
static bool increasing_saturation_LED1 = true;

static uint32_t value_duty_LED1 = 0;

static void hsv_to_rgb_float();
static void change_value_smoothly(uint32_t *value, bool *increasing, uint32_t min_value, uint32_t max_value, uint32_t step);

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

HSB_color HSB_current_state = {
    .hue = (uint32_t)360 * 0.85,
    .saturation = SATURATION_TOP_VALUE,
    .brightness = BRIGHTNESS_TOP_VALUE};

HSB_color HSB_save = {
    .hue = (uint32_t)360 * 0.85,
    .saturation = SATURATION_TOP_VALUE,
    .brightness = BRIGHTNESS_TOP_VALUE};

void init_state_RGB(void)
{
    HSB_current_state = HSB_save;
    uint32_t read = nvmc_read_last_data((uint32_t *)&(HSB_save));
    if (read > 0)
    {
        HSB_current_state = HSB_save;
    }
}

static void blinky_save_data(void)
{

    NRF_LOG_INFO("Start save data");

    if ((abs(HSB_save.hue - HSB_current_state.hue) < 1.f) &&
        (abs(HSB_save.saturation - HSB_current_state.saturation) < 1.f) &&
        (abs(HSB_save.brightness - HSB_current_state.brightness) < 1.f))
    {
        NRF_LOG_INFO("CURRENT STATE -> Hue: %d; Saturation: %d; Brightness: %d", HSB_current_state.hue, HSB_current_state.saturation, HSB_current_state.brightness);
        NRF_LOG_INFO("SAVE STATE -> Hue: %d; Saturation: %d; Brightness: %d", HSB_save.hue, HSB_save.saturation, HSB_save.brightness);
        NRF_LOG_INFO("Nothing save");
        return;
    }

    nvmc_write_data((uint32_t *)&(HSB_current_state));
    while (!nvmc_write_complete_check()) // можно убрать
    {
    }

    NRF_LOG_INFO("End save data");
    HSB_save = HSB_current_state;

    NRF_LOG_INFO("CURRENT STATE -> Hue: %d; Saturation: %d; Brightness: %d", HSB_current_state.hue, HSB_current_state.saturation, HSB_current_state.brightness);
    NRF_LOG_INFO("NEW SAVE STATE -> Hue: %d; Saturation: %d; Brightness: %d", HSB_save.hue, HSB_save.saturation, HSB_save.brightness);
}

void set_current_mode(void)
{
    current_mode = (current_mode + 1) % 4;
    NRF_LOG_INFO("Current mode: %s", controller_mode_strings[(int)current_mode]);
    if (current_mode == MODE_AFK)
    {
        blinky_save_data();
    }
}

void update_value_HSB(void)
{

    NRF_LOG_INFO("Hue: %d; Saturation: %d; Brightness: %d", HSB_current_state.hue, HSB_current_state.saturation, HSB_current_state.brightness);

    switch (current_mode)
    {
    case MODE_HUE:
        HSB_current_state.hue = (HSB_current_state.hue + 1) % 360;
        break;

    case MODE_SATURATION:
        change_value_smoothly(&HSB_current_state.saturation, &increasing_saturation, 0, SATURATION_TOP_VALUE, SATURATION_STEP);
        break;

    case MODE_BRIGHTNESS:
        change_value_smoothly(&HSB_current_state.brightness, &increasing_brightness, 0, BRIGHTNESS_TOP_VALUE, BRIGHTNESS_STEP);
        break;

    default:
        break;
    }
}

void update_value_LED1(void)
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

static void change_value_smoothly(uint32_t *value, bool *increasing, uint32_t min_value, uint32_t max_value, uint32_t step)
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
    float hue = HSB_current_state.hue % 360;
    float saturation = HSB_current_state.saturation / 100.0f;
    float value = HSB_current_state.brightness / 100.0f;

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

/**
 * @brief Преобразование RGB в HSV
 * @param r значение красного (0-255)
 * @param g значение зеленого (0-255)
 * @param b значение синего (0-255)
 * @param h указатель для сохранения оттенка (0-360)
 * @param s указатель для сохранения насыщенности (0-100)
 * @param v указатель для сохранения яркости (0-100)
 */
static void rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b,
                       uint32_t *h, uint32_t *s, uint32_t *v)
{
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    float cmax = rf;
    if (gf > cmax)
        cmax = gf;
    if (bf > cmax)
        cmax = bf;

    float cmin = rf;
    if (gf < cmin)
        cmin = gf;
    if (bf < cmin)
        cmin = bf;

    float delta = cmax - cmin;

    // Вычисление H
    if (delta == 0)
    {
        *h = 0;
    }
    else if (cmax == rf)
    {
        *h = (uint32_t)(60.0f * fmodf(((gf - bf) / delta), 6));
    }
    else if (cmax == gf)
    {
        *h = (uint32_t)(60.0f * ((bf - rf) / delta + 2));
    }
    else
    {
        *h = (uint32_t)(60.0f * ((rf - gf) / delta + 4));
    }

    // Вычисление S
    *s = (cmax == 0) ? 0 : (uint32_t)((delta / cmax) * 100);

    // Вычисление V
    *v = (uint32_t)(cmax * 100);
}

void led_set_rgb_color(uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t h, s, v;
    rgb_to_hsv(red, green, blue, &h, &s, &v);
    HSB_current_state.hue = h;
    HSB_current_state.saturation = s;
    HSB_current_state.brightness = v;
}

void led_set_hsv_color(uint32_t hue, uint32_t saturation, uint32_t value)
{
    HSB_current_state.hue = hue;
    HSB_current_state.saturation = saturation;
    HSB_current_state.brightness = value;
}
