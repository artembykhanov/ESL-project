#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#define LED_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_B_PIN NRF_GPIO_PIN_MAP(0, 12)


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
    uint32_t hue;
    uint32_t saturation;
    uint32_t brightness;
} HSB_color;

void set_current_mode(void);
void update_value_HSB(void);
void update_value_LED1(void);
void led_display_current_color(void);

void init_led_pin(void);
void turn_on_led(int pin);
void turn_off_led(int pin);
void turn_off_all_leds(void);

void init_state_RGB(void);

/**
 * @brief Установка цвета в формате RGB
 * @param red значение красного (0-255)
 * @param green значение зеленого (0-255)
 * @param blue значение синего (0-255)
 */
void led_set_rgb_color(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Установка цвета в формате HSV
 * @param hue оттенок (0-360)
 * @param saturation насыщенность (0-100)
 * @param value яркость (0-100)
 */
void led_set_hsv_color(uint32_t hue, uint32_t saturation, uint32_t value);

#endif // LED_CONTROL_H
