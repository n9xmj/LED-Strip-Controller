/*
 * app_main.c
 *
 * Created on: Apr 26, 2026
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "debug_menu.h"
#include "led_strip_control.h"

/******************************************************************************
 * __io_putchar() and __io_getchar() definitions here override the weak
 * definitions made somewhere in the libc code.
 *
 * The definitions here route STDxxx stream I/O (e.g. putchar, getchar, printf,
 * etc.) to the debug port UART.
 *
 * See also: syscalls.c and syscalls.h
 ******************************************************************************/

int __io_putchar(int ch)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART1 and Loop until the end of transmission */
    uint8_t u8_ch = ch;
    HAL_UART_Transmit(&DEBUG_UART_HANDLE, &u8_ch, 1, 0xFFFF);

    return ch;
}

int __io_getchar(void)
{
    uint8_t u8_ch;
    HAL_StatusTypeDef x_status;
    x_status = HAL_UART_Receive(&DEBUG_UART_HANDLE, &u8_ch, 1, 1);
    if (x_status == HAL_OK)
    {
        return (int) u8_ch;
    }

    return 0;
}

/******************************************************************************
 * HAL UART callbacks (application-owned). Forward LED strip UART/DMA events;
 * add other UART users here when needed.
 ******************************************************************************/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    led_strip_uart_tx_complete(huart);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    led_strip_uart_error(huart);
}

/******************************************************************************
 *
 ******************************************************************************/

void v_print_project_banner(void)
{
    printf("*** " PROJECT_NAME " ***\r\n");
}

/******************************************************************************
 *
 ******************************************************************************/

static void v_system_init(void)
{
    // Nothing to do yet
}

/******************************************************************************
 *
 ******************************************************************************/

void v_app_polling_task(void)
{
    // Nothing here yet
}

__attribute__((noreturn)) void v_app_main(void)
{
    // Initialize any hardware, APIs, etc.
    v_system_init();

    // Print out banner sign-on message for this project
    v_print_project_banner();

    // Initialize debug menu
    v_debug_menu_init();

    while (1)
    {
        v_app_polling_task();
        v_debug_menu_service();
    }
}
