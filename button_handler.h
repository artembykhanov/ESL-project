#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <stdbool.h>
#include "nrfx_gpiote.h"

#define BUTTON_PIN NRF_GPIO_PIN_MAP(1, 6)

typedef void (*click_callback)(void);

/**
 * @brief Инициализация обработчика кнопок
 *
 * @param on_single_click Колбэк для одиночного клика
 * @param on_double_click Колбэк для двойного клика
 * @param on_long_press   Колбэк для долгого нажатия
 */
void button_init(click_callback on_single_click, click_callback on_double_click, click_callback on_long_press);

#endif // BUTTON_HANDLER_H