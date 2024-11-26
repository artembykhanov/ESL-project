#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#define APP_TIMER_REPEATED_MS 30

#define LED_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_B_PIN NRF_GPIO_PIN_MAP(0, 12)

#define PWM_TOP_VALUE 255

void pwm_controller_init(void);
void pwm_timer_start(void);
void pwm_start_playback(void);
void pwm_update_duty_cycle(uint8_t channel, uint32_t duty_cycle);

#endif // PWM_CONTROL_H
