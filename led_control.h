#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#define LED_TURN_OFF 1
#define LED_TURN_ON 0

#define LED_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_B_PIN NRF_GPIO_PIN_MAP(0, 12)

#define SATURATION_STEP 3
#define BRIGHTNESS_STEP 3

typedef enum
{
    MODE_AFK,
    MODE_HUE,
    MODE_SATURATION,
    MODE_BRIGHTNESS
} controller_mode_t;

typedef struct
{
    uint32_t afk_mode;
    uint32_t hue_mode;
    uint32_t saturation_mode;
    uint32_t brightness_mode;
} steps_mode;

void set_current_mode(void);
void update_duty_cycle_RGB(void);
void update_duty_cycle_LED1(void);
void change_value_smoothly(uint32_t *value, bool *increasing, uint32_t min_value, uint32_t max_value, uint32_t step);
void hsv_to_rgb_float(uint32_t h, uint32_t s, uint32_t v, uint8_t *r, uint8_t *g, uint8_t *b);
void led_display_current_color(void);

void init_led_pin(void);
void turn_on_led(int pin);
void turn_off_led(int pin);
void turn_off_all_leds(void);

#endif // LED_CONTROL_H
