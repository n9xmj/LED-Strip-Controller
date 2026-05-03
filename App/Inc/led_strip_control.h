/**
 * @file led_strip_control.h
 * @author Mark Schultz (n9xmj@yahoo.com)
 * @brief Public API for WS2812 / SK6812 strip control (USART + DMA).
 * @version 0.1
 * @date 2026-05-02
 * 
 * @copyright None/TBD/whatever
 * 
 */

#pragma once

#include "platform.h"

// LED strip types

typedef enum
{
    LED_STRIP_TYPE_UNKNOWN,
    LED_STRIP_TYPE_WS2812,
    LED_STRIP_TYPE_SK6812,
}
led_strip_type_t;

// Color channels for WS2812/SK6812
// Use these when accessing color channels via ws_pixel_t.x_color[]

typedef enum __attribute__((packed))
{
    LED_COLOR_GREEN,
    LED_COLOR_RED,
    LED_COLOR_BLUE,
    LED_COLOR_WHITE
}
ws_color_t;

// WS2812/SK6812 pixel (individual LED)
// White channel is not used on WS2812 (SK6812 only)

typedef union
{
    ws_color_t  x_color[4];          // Access color channels via array
    uint32_t    u32_all;             // Entire struct content in a uint32_t
    struct
    {
        uint8_t u8_green;           // Green channel
        uint8_t u8_red;             // Red channel
        uint8_t u8_blue;            // Blue channel
        uint8_t u8_white;           // White channel - SK6812 only
    };
}
ws_pixel_t;

// LED strip control struct (handle)

typedef struct
{
    UART_HandleTypeDef      *p_x_uart;  // HAL handle for U(S)ART to use
    DMA_HandleTypeDef       *p_x_dma;   // HAL handle for U(S)ART DMA channel

    ws_pixel_t  *p_x_pixel;             // Pixel buffer
    uint8_t     *p_u8_dma_buffer;       // DMA buffer for UART

    uint16_t    u16_strip_length;       // Number of pixels (LEDs) in the strip

    bool        b_initialized;          // true: Strip instance has been init'd and is active
    bool        b_pixel_malloc;         // true: p_x_pixel buffer was auto-malloc'd
    bool        b_dma_malloc;           // true: p_u8_dma_buffer was auto-malloc'd

    bool        b_transfer_in_progress; // true when strip DMA transfer is in progress; blocking flag
}
led_strip_handle_t;

//------------------------------------------------------------------------------

/*
 * Need public API functions for:
 * - Create/initialize a strip instance
 *   Passed in:
 *     -> pointer to uninit'd led_strip_handle_t instance - caller must instantiate
 *     -> uart handle
 *     -> dma handle - can this be auto-identified?
 *     -> strip length,
 *     -> pixel buf ptr - malloc if NULL
 *     -> dma buf ptr - malloc if NULL
 *   Returns:
 * - Destroy a strip instance
 *   Blocks if transfer in progress, w/timeout
 *   Passed in:
 *   -> ptr to led_strip_handle_t
 *   Returns: void (?)
 * - Initiate data transfer to LED strip
 *   blocks if transfer in progress, w/timeout
 *   Passed in: ptr to led_strip_handle_t
 * - Check transfer progress
 *   Passed in: ptr to led_strip_handle_t
 *   Returns: led_strip_handle_t.b_transfer_in_progress
 * - Other API functions TBD
 *
 * Private functions:
 * - TBD
 *
 * Implementation notes:
 * - No need for opaque structs or tight encapsulation; a "reasonable amount" is satisfactory.
 */
