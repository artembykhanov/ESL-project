#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#define PWM_TOP_VALUE 255

void pwm_controller_init(void);
void pwm_timer_start(void);
void pwm_start_playback(void);
void pwm_update_duty_cycle(uint8_t channel, uint32_t duty_cycle);

#endif // PWM_CONTROL_H
