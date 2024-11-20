#include "led_control.h"
#include "pwm_control.h"
#include "button_handler.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf_drv_clock.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"

void init_logs(void);
void lfclk_request(void);
void init_helper(void);

void blinky_on_button_click(void)
{
}

void blinky_on_button_double_click(void)
{
    set_current_mode();
}

void blinky_on_button_long_press(void)
{
    update_duty_cycle_RGB();
}

int main(void)
{
    init_logs();
    NRF_LOG_INFO("Starting the main application");
    lfclk_request();

    init_helper();

    turn_off_all_leds();

    pwm_start_playback();
    pwm_timer_start();

    while (true)
    {
        __WFI();
    }
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

void init_helper(void)
{
    button_pin_init();
    button_timers_init();
    button_event_init(blinky_on_button_click, blinky_on_button_double_click, blinky_on_button_long_press);

    pwm_timer_init();
    pwm_controller_init();
}