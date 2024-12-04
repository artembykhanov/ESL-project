#ifndef NVMC_CONTROL_H
#define NVMC_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Инициализация NVMC
 * @param writable_block_size Размер блока в байтах
 */
void nvmc_initialize(uint32_t writable_block_size);

/**
 * @brief Чтение последнего записанного блока
 * @param buffer  Буфер для чтения данных
 * @return Размер прочитанных данных в байтах, 0 если чтение не удалось
 */
uint32_t nvmc_read_last_data(uint32_t *buffer);

/**
 * @brief Запись блока данных
 * @param data  Данные для записи
 */
void nvmc_write_data(uint32_t *data);

/**
 * @brief Проверка завершения записи
 * @return true - запись завершена, false - запись продолжается
 */
bool nvmc_write_complete_check(void);

#endif /* NVMC_CONTROL_H */
