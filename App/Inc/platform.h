/**
 * @file platform.h
 * @author Mark Schultz (n9xmj@yahoo.com)
 * @brief Platform-specific definitions and macros
 * @version 0.1
 * @date 2026-04-27
 * 
 * @copyright None/TBD/whatever
 * 
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_exti.h"

#include "main.h"

 //------------------------------------------------------------------------------
 // Stringification
 //------------------------------------------------------------------------------
 
// This is a helper macro for VSTR, it is not much use when used directly
// If you use STR(MACRONAME) in your code, you'll get the macro NAME in
// string form: "MACRONAME"

#define STR(s) #s
 
// Use VSTR(macroname) to get the -value- of <macroname> in quoted-string form
// e.g. if you created this #define:
// #define FOO 1234
// and then use VSTR(FOO), you get the expanded value of FOO in string form
// in your code:
// "1234"
 
#define VSTR(s) STR(s)
 
 //------------------------------------------------------------------------------
 // Compiler/toolchain specific
 //------------------------------------------------------------------------------

#define PACKED          __attribute__((packed))
#define MAYBE_UNUSED    __attribute__((unused))
#define NEVER_RETURNS   __attribute__((noreturn))

#define DISABLE_INTERRUPTS(primask) \
    primask = __get_PRIMASK(); \
    __disable_irq();

#define RESTORE_INTERRUPTS(primask) \
    __set_PRIMASK(primask);

#define ATOMIC_BLOCK_BEGIN \
do { \
    uint32_t __primask = __get_PRIMASK(); \
    __disable_irq();

#define ATOMIC_BLOCK_END \
    __set_PRIMASK(__primask); \
} while (0);

 // BM2N(mask) : Bitmask-to-number
 // Returns number of trailing binary zeroes in <mask>
 // 0x0010 0000 -> 20 : the least significant 20 bits are 0
 // 0xFFF0 0000 -> 20 : bits 'above' the least significant 1-bit are don't-care
 // 0x0000 0001 -> 0
 // 0xF000 8000 -> 15
 // 0x8000 0000 -> 31
 #define BM2N(mask)     (__builtin_ctzl((uint32_t) (mask)))
 
 // Convert a GPIO register block address (e.g. GPIOA, GPIOB, GPIOC, etc.)
 // to a GPIO port number (e.g. 0=GPIOA, 1=GPIOB, 2=GPIOC, etc.)
 //
 // if (reg) is a constant that can be resolved at compile-time, then this
 // macro will not generate any code; it will resolve to a compile-time constant.
 // (at least if one is using GCC)
 
 #define GPIO_ADDR_TO_PORT_NUM(reg) \
     ( ((reg) == GPIOA) ? 0 \
     : ((reg) == GPIOB) ? 1 \
     : ((reg) == GPIOC) ? 2 \
     : ((reg) == GPIOD) ? 3 \
 /*    : ((reg) == GPIOE) ? 4 */ \
     : ((reg) == GPIOF) ? 5 \
 /*    : ((reg) == GPIOG) ? 6 */ \
 /*    : ((reg) == GPIOH) ? 7 */ \
 /*    : ((reg) == GPIOI) ? 8 */ \
 /*    : ((reg) == GPIOJ) ? 9 */ \
 /*    : ((reg) == GPIOK) ? 10 */ \
     : 15 )
 
 //------------------------------------------------------------------------------
 // Misc. constants
 //------------------------------------------------------------------------------
 
 #define US_IN_1S    1000000             // # microseconds in 1 second
 #define MS_IN_1S    1000                // # milliseconds in 1 second
 
 #define ELAPSED_TIME(ts)    (HAL_GetTick() - (ts))
 
//------------------------------------------------------------------------------
// MCU peripheral / IP block assignments
//------------------------------------------------------------------------------
 
#define DELAY_US_TIMER_HANDLE       htim7       // Timer used for v_delay_us(); exclusive use

#define DEBUG_UART_HANDLE           huart2      // UART used for debug output

#define LED_CHANNEL_1_UART_HANDLE   huart1
#define LED_CHANNEL_2_UART_HANDLE   huart3
#define LED_CHANNEL_3_UART_HANDLE   huart4

#define LCD_SPI_HANDLE              hspi1

//------------------------------------------------------------------------------
// GPIO control macros
//------------------------------------------------------------------------------

// Control the user LED on STM32 Nucleo board
#define NUCLEO_LED_ON()             HAL_GPIO_WritePin(NUCLEO_LED_GPIO_Port, NUCLEO_LED_Pin, 1)
#define NUCLEO_LED_OFF()            HAL_GPIO_WritePin(NUCLEO_LED_GPIO_Port, NUCLEO_LED_Pin, 0)
#define NUCLEO_LED_SET(level)       HAL_GPIO_WritePin(NUCLEO_LED_GPIO_Port, NUCLEO_LED_Pin, (level))

// Read the state of the user button on STM32 Nucleo board
#define NUCLEO_BUTTON_PRESSED()     (HAL_GPIO_ReadPin(NUCLEO_BUTTON_GPIO_Port, NUCLEO_BUTTON_Pin) == 0)
