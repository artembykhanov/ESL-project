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
} controller_mode;

typedef struct
{
    uint16_t afk_const;
    uint16_t hue_step;
    uint16_t saturation_step;
    uint16_t brightness_const;
} mode_steps;

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB_color;

typedef struct
{
    uint16_t hue;
    uint16_t saturation;
    uint16_t brightness;
} HSB_color;


void set_current_mode(void);
void update_duty_cycle_RGB(void);
void update_duty_cycle_LED1(void);
void led_display_current_color(void);

void init_led_pin(void);
void turn_on_led(int pin);
void turn_off_led(int pin);
void turn_off_all_leds(void);

#endif // LED_CONTROL_H
