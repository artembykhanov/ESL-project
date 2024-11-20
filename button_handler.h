#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <stdbool.h>
#include "nrfx_gpiote.h"

#define BUTTON_PIN NRF_GPIO_PIN_MAP(1, 6)
#define DEBOUNCE_INTERVAL APP_TIMER_TICKS(50)
#define DOUBLE_CLICK_INTERVAL APP_TIMER_TICKS(500)
#define LONG_PRESS_INITIAL_INTERVAL APP_TIMER_TICKS(1000) 
#define LONG_PRESS_REPEAT_INTERVAL APP_TIMER_TICKS(30)     

typedef void (*click_callback)(void);

void button_event_init(click_callback on_single_click, click_callback on_double_click, click_callback on_long_press);
void button_pin_init(void);
void button_timers_init(void);
bool is_button_pressed(void);

void debounce_timer_handler(void *p_context);
void double_click_timer_handler(void *p_context);
void long_press_timer_handler(void *p_context);
void long_press_repeat_timer_handler(void *p_context);

void button_interrupt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void process_button_click(void);

#endif // BUTTON_HANDLER_H