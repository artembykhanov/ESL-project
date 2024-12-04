#include <stdbool.h>
#include <stdint.h>

#include "led_control.h"
#include "pwm_control.h"
#include "button_handler.h"

#include "nvmc_control.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"

void init_logs(void);
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
    update_value_HSB();
}

int main(void)
{
    init_logs();
    NRF_LOG_INFO("Starting the main application");

    init_helper();

    turn_off_all_leds();

    pwm_start_playback();
    pwm_timer_start();

    while (true)
    {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
        __WFI();
    }
}

void init_logs(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void init_helper(void)
{
    button_init(blinky_on_button_click, blinky_on_button_double_click, blinky_on_button_long_press);

    pwm_controller_init();

    nvmc_initialize(sizeof(HSB_color));
    init_state_RGB();
}