#include "nvmc_control.h"

#include "nrfx_nvmc.h"
#include "nrf_bootloader_info.h"
#include "nrf_dfu_types.h"

#include "nrf_log.h"

#define NVMC_BOOTLOADER_START_ADDR (0xE0000)
#define NVMC_PAGE_SIZE (0x1000)
#define NVMC_PAGE_START (NVMC_BOOTLOADER_START_ADDR - NRF_DFU_APP_DATA_AREA_SIZE)
#define NVMC_EMPTY_VALUE 0xFFFFFFFF
#define NVMC_WORD_SIZE (sizeof(uint32_t))

// Структура контекста
typedef struct
{
    bool erase_needed;
    uint32_t writable_block_size;
    uint32_t *current_address;
} nvmc_context_t;

static nvmc_context_t nvmc_context = {
    .erase_needed = false,
    .current_address = NULL,
    .writable_block_size = 0};

/**
 * @brief Чтение одного слова из памяти
 * @param addr Адрес начала чтения
 * @param data Указатель на переменную для прочитанного слова
 */
static void nvmc_read_word(uint32_t *addr, uint32_t *data)
{
    *data = *addr;
}

/**
 * @brief Запись одного слова в память
 * @param addr Адрес начала записи
 * @param data Данные для записи
 */
static void nvmc_write_word(uint32_t *addr, uint32_t data)
{
    nrfx_nvmc_word_write((uint32_t)addr, data);
}

/**
 * @brief Стирание страницы памяти
 * @param page_start_addr Адрес начала страницы
 * @return NRFX_SUCCESS если стирание успешно, иначе код ошибки
 */
static nrfx_err_t nvmc_erase_page(uint32_t *page_start_addr)
{
    return nrfx_nvmc_page_erase((uint32_t)page_start_addr);
}

/**
 * @brief Поиск адреса последнего записанного блока
 * @param result Указатель для сохранения найденного адреса
 * @return true если нашли  блок, false если страница заполнена
 */
static bool nvmc_find_last_address(uint32_t *result)
{
    uint32_t *addr = (uint32_t *)NVMC_PAGE_START;
    uint32_t *last_valid_addr = NULL;
    uint32_t *page_end = (uint32_t *)(NVMC_PAGE_START + NVMC_PAGE_SIZE);

    while (addr < page_end)
    {
        uint32_t block_size;
        nvmc_read_word(addr, &block_size);

        if (block_size == NVMC_EMPTY_VALUE)
        {
            *result = (uint32_t)last_valid_addr;    
            return true;
        }

        last_valid_addr = addr;
        addr += 1 + block_size / NVMC_WORD_SIZE;
    }

    nvmc_context.erase_needed = true;
    return false;
}

void nvmc_initialize(uint32_t writable_block_size)
{
    nvmc_context.writable_block_size = writable_block_size;
    nvmc_context.current_address = (uint32_t *)NVMC_PAGE_START;
}

uint32_t nvmc_read_last_data(uint32_t *buffer)
{
    uint32_t last_address = 0;

    if (!nvmc_find_last_address(&last_address))
    {
        return 0;
    }

    uint32_t block_size;
    uint32_t *addr = (uint32_t *)last_address;
    nvmc_read_word(addr, &block_size);

    if (block_size != nvmc_context.writable_block_size)
    {
        nvmc_context.erase_needed = true;
        return 0;
    }

    nvmc_context.current_address = addr + 1;
    for (uint32_t i = 0; i < nvmc_context.writable_block_size / NVMC_WORD_SIZE; ++i)
    {
        nvmc_read_word(nvmc_context.current_address++, &buffer[i]);
    }

    return nvmc_context.writable_block_size;
}

void nvmc_write_data(uint32_t *data)
{
    if (nvmc_context.erase_needed ||
        ((uint32_t)nvmc_context.current_address + nvmc_context.writable_block_size + NVMC_WORD_SIZE >
         NVMC_PAGE_START + NVMC_PAGE_SIZE))
    {
        nvmc_erase_page((uint32_t *)NVMC_PAGE_START);

        nvmc_context.erase_needed = false;
        nvmc_context.current_address = (uint32_t *)NVMC_PAGE_START;
    }

    // Записываем размер блока
    nvmc_write_word(nvmc_context.current_address++, nvmc_context.writable_block_size);

    // Записываем данные
    for (uint32_t i = 0; i < nvmc_context.writable_block_size / NVMC_WORD_SIZE; ++i)
    {
        nvmc_write_word(nvmc_context.current_address++, data[i]);
    }
}

bool nvmc_write_complete_check(void)
{
    return nrfx_nvmc_write_done_check();
}
