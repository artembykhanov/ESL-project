#include "pwm_control.h"
#include "led_control.h"

#include "nrf_gpio.h"
#include "nrfx_pwm.h"

#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

APP_TIMER_DEF(timer_pwm);

static nrfx_pwm_t rgb_instance = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_t led_instance = NRFX_PWM_INSTANCE(1);

static nrf_pwm_values_individual_t pwm_duty_cycles;
static nrf_pwm_sequence_t const pwm_sequence =
    {
        .values.p_individual = &pwm_duty_cycles,
        .length = NRF_PWM_VALUES_LENGTH(pwm_duty_cycles),
        .repeats = 0,
        .end_delay = 0};

void pwm_controller_init(void)
{
    nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
    pwm_config.output_pins[0] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.output_pins[1] = LED_R_PIN | NRFX_PWM_PIN_INVERTED;
    pwm_config.output_pins[2] = LED_G_PIN | NRFX_PWM_PIN_INVERTED;
    pwm_config.output_pins[3] = LED_B_PIN | NRFX_PWM_PIN_INVERTED;
    pwm_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    pwm_config.top_value = PWM_TOP_VALUE;

    nrfx_pwm_config_t led_config = NRFX_PWM_DEFAULT_CONFIG;
    led_config.output_pins[0] = LED_PIN | NRFX_PWM_PIN_INVERTED;
    led_config.output_pins[1] = NRFX_PWM_PIN_NOT_USED;
    led_config.output_pins[2] = NRFX_PWM_PIN_NOT_USED;
    led_config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
    led_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    led_config.top_value = PWM_TOP_VALUE;

    nrfx_pwm_init(&rgb_instance, &pwm_config, NULL);
    nrfx_pwm_init(&led_instance, &led_config, NULL);
}

void pwm_timer_handler(void *p_context)
{
    led_display_current_color();
    update_duty_cycle_LED1();
}

void pwm_timer_init(void)
{
    app_timer_init();
    app_timer_create(&timer_pwm, APP_TIMER_MODE_REPEATED, pwm_timer_handler);
}

void pwm_timer_start(void)
{
    app_timer_start(timer_pwm, APP_TIMER_TICKS(APP_TIMER_REPEATED_MS), NULL);
}

void pwm_start_playback(void)
{
    nrfx_pwm_simple_playback(&rgb_instance, &pwm_sequence, 1, NRFX_PWM_FLAG_LOOP);
    nrfx_pwm_simple_playback(&led_instance, &pwm_sequence, 1, NRFX_PWM_FLAG_LOOP);
}

void pwm_update_duty_cycle(uint8_t channel, uint32_t duty_cycle)
{
    duty_cycle %= PWM_TOP_VALUE + 1;

    switch (channel)
    {
    case 0:
        pwm_duty_cycles.channel_0 = duty_cycle;
        break;
    case 1:
        pwm_duty_cycles.channel_1 = duty_cycle;
        break;
    case 2:
        pwm_duty_cycles.channel_2 = duty_cycle;
        break;
    case 3:
        pwm_duty_cycles.channel_3 = duty_cycle;
        break;
    }
}