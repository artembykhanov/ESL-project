/**
 * @brief Модуль управления CLI (Command Line Interface) через USB CDC ACM
 * 
 * Модуль обеспечивает:
 * - Прием и обработку команд через USB
 * - Эхо введенных символов
 * - Поддержку команд RGB и HSV для управления цветом
 * - Валидацию введенных значений
 */

#include "cli_control.h"
#include "led_control.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_usbd.h"
#include "app_usbd_cdc_acm.h"

#define READ_SIZE 1
#define MAX_CMD_SIZE 64

static char m_rx_buffer[READ_SIZE];
static char m_cmd_buffer[MAX_CMD_SIZE];
static uint8_t m_cmd_pos = 0;

static void process_command(void);
static void send_response(const char *str);
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const *p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE 2
#define CDC_ACM_COMM_EPIN NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE 3
#define CDC_ACM_DATA_EPIN NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT NRF_DRV_USBD_EPOUT4

APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const *p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    switch (event)
    {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
    {
        ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm, m_rx_buffer, READ_SIZE);
        UNUSED_VARIABLE(ret);
        NRF_LOG_INFO("CDC ACM port opened");
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
    {
        NRF_LOG_INFO("CDC ACM port closed");
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
    {
        NRF_LOG_DEBUG("TX done");
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
    {
        ret_code_t ret;
        do
        {
            // Проверка на символы конца строки
            if (m_rx_buffer[0] == '\r' || m_rx_buffer[0] == '\n')
            {
                if (m_cmd_pos > 0)
                {
                    // Завершаем строку и обрабатываем команду
                    m_cmd_buffer[m_cmd_pos] = '\0';
                    NRF_LOG_INFO("Received command: %s", m_cmd_buffer);
                    process_command();
                    m_cmd_pos = 0;
                }
                app_usbd_cdc_acm_write(&m_app_cdc_acm, "\r\n", 2);
            }
            else
            {
                // Накапливаем символы команды и отправляем эхо
                if (m_cmd_pos < MAX_CMD_SIZE - 1)
                {
                    m_cmd_buffer[m_cmd_pos++] = m_rx_buffer[0];
                    app_usbd_cdc_acm_write(&m_app_cdc_acm, m_rx_buffer, 1);
                }
                else
                {
                    NRF_LOG_WARNING("Command buffer overflow");
                    m_cmd_pos = 0;
                }
            }

            ret = app_usbd_cdc_acm_read(&m_app_cdc_acm, m_rx_buffer, READ_SIZE);
        } while (ret == NRF_SUCCESS);
        break;
    }
    default:
        break;
    }
}

static void process_command(void)
{
    char response[128];
    char *cmd = strtok(m_cmd_buffer, " ");

    if (cmd == NULL)
    {
        NRF_LOG_WARNING("Empty command received");
        send_response("\r\nUnknown command\r\n");
        return;
    }

    char cmd_upper[MAX_CMD_SIZE];
    strncpy(cmd_upper, cmd, MAX_CMD_SIZE - 1);
    cmd_upper[MAX_CMD_SIZE - 1] = '\0';
    for (int i = 0; cmd_upper[i]; i++)
    {
        cmd_upper[i] = toupper(cmd_upper[i]);
    }

    if (strcmp(cmd_upper, "HELP") == 0)
    {
        NRF_LOG_INFO("Processing help command");
        send_response(
            "\r\n"
            "Commands:\r\n"
            "RGB <r> <g> <b> - r - red [0..255], g - green [0..255], b - blue [0..255]\r\n"
            "HSV <h> <s> <v> - h - hue [0..360], s - saturation [0..100], v - value/brightness [0..100]\r\n"
            "help - show this message\r\n");
    }
    else if (strcmp(cmd_upper, "RGB") == 0)
    {
        char *r_str = strtok(NULL, " ");
        char *g_str = strtok(NULL, " ");
        char *b_str = strtok(NULL, " ");

        if (r_str && g_str && b_str)
        {
            int r = atoi(r_str);
            int g = atoi(g_str);
            int b = atoi(b_str);
            
            if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255)
            {
                NRF_LOG_INFO("Setting RGB color: R=%d G=%d B=%d", r, g, b);
                led_set_rgb_color((uint8_t)r, (uint8_t)g, (uint8_t)b);
  
                snprintf(response, sizeof(response),
                         "\r\nColor set to R=%d G=%d B=%d\r\n", r, g, b);
                send_response(response);
            }
            else
            {
                NRF_LOG_WARNING("Invalid RGB values: R=%d G=%d B=%d", r, g, b);
                send_response("\r\nInvalid RGB values (each should be 0-255)\r\n");
            }
        }
        else
        {
            NRF_LOG_WARNING("Invalid RGB command format received");
            send_response("\r\nInvalid RGB command format\r\n");
        }
    }
    else if (strcmp(cmd_upper, "HSV") == 0)
    {
        char *h_str = strtok(NULL, " ");
        char *s_str = strtok(NULL, " ");
        char *v_str = strtok(NULL, " ");

        if (h_str && s_str && v_str)
        {
            uint32_t h = (uint32_t)atoi(h_str);
            uint32_t s = (uint32_t)atoi(s_str);
            uint32_t v = (uint32_t)atoi(v_str);

            if (h <= 360 && s <= 100 && v <= 100)
            {
                NRF_LOG_INFO("Setting HSV color: H=%d S=%d V=%d", h, s, v);
                led_set_hsv_color(h, s, v);

                snprintf(response, sizeof(response),
                         "\r\nColor set to H=%ld S=%ld V=%ld\r\n", h, s, v);
                send_response(response);
            }
            else
            {
                NRF_LOG_WARNING("Invalid HSV values: H=%d S=%d V=%d", h, s, v);
                send_response("\r\nInvalid HSV values (H:0-360, S/V:0-100)\r\n");
            }
        }
        else
        {
            NRF_LOG_WARNING("Invalid HSV command format received");
            send_response("\r\nInvalid HSV command format\r\n");
        }
    }
    else
    {
        NRF_LOG_WARNING("Unknown command received: %s", cmd);
        snprintf(response, sizeof(response), 
                 "\r\nUnknown command: %s\r\n", cmd);
        send_response(response);
    }
}

/**
 * @brief Отправка ответа пользователю
 * 
 * @param str Строка для отправки
 */
static void send_response(const char *str)
{
    ret_code_t ret;

    ret = app_usbd_cdc_acm_write(&m_app_cdc_acm, (uint8_t *)str, strlen(str));

    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Failed to send response");
    }
    else
    {
        NRF_LOG_DEBUG("Sending response: %s", str);
    }
}

/**
 * @brief Инициализация CLI интерфейса
 * 
 * Инициализирует CDC ACM класс USB и подготавливает интерфейс к работе
 */
void cli_init(void)
{
    NRF_LOG_INFO("Initializing CLI interface");
    app_usbd_class_inst_t const *class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Failed to initialize CLI interface, error: %d", ret);
    }
    APP_ERROR_CHECK(ret);
    NRF_LOG_INFO("CLI interface initialized successfully");
}

/**
 * @brief Обработка событий CLI
 */
void cli_process(void)
{
    while (app_usbd_event_queue_process())
    {
    }
}