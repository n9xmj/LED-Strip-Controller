STM32 WS2812 / SK6812 LED Strip Driver Project

For Cursor / Claude / other AI coding assistants

1, PROJECT OVERVIEW AND GOALS (SHORT-TERM FOCUS)

The primary goal is to develop a clean, robust, extensible, and STM32-platform-independent API for driving WS2812 (RGB) and SK6812 (RGBW) LED strings using the USART data-line-inversion trick.

Short-term priority:
- DMA-driven USART transmission (zero-CPU updates where possible)
- High-level, easy-to-use API
- Make the driver portable across different STM32 families

Key requirements:
- Use STM32Cube ecosystem + STM32CubeIDE + GCC toolchain (no Arduino/Wiring)
- Prefer STM32Cube HAL for high-level functionality (especially USART + DMA)
- STM32 LL (Low Layer) is acceptable for performance-critical sections if it remains reasonably portable
- Support for multiple independent LED strips on different USARTs using the same API

2. HARDWARE CONTEXT

Primary development platform: STM32G0B1RE Nucleo (user is most familiar with the G0Bx series)
Other boards available for later testing (STM32F401 Black Pill, G0B0, U083, H573, etc.)

Debug / Console Port
One UART (typically USART2, pre-wired to ST-Link VCP on the Nucleo) will be used as a console debug port. A user-provided menu system will be integrated for runtime interaction and debugging.

3. USART DRIVING TECHNIQUE

We use the well-known STM32 USART trick:
- TXINV = 1 (data line inverted)
- 7-bit word length
- Baud rate approx 2.4 Mbaud (416.67 ns per UART bit)
- One UART frame (9 bit-times) encodes exactly three WS2812/SK6812 sub-bits
This gives clean, low-jitter timing with excellent DMA support.

4. DATA STRUCTURES

typedef struct __attribute__((packed))
{
    uint8_t green;
    uint8_t red;
    uint8_t blue;
    uint8_t white;   // set to 0 for WS2812-only strings
}
ws2812_led_pixel_t;

typedef enum
{
    LED_TYPE_WS2812 = 24,   // GRB
    LED_TYPE_SK6812 = 32    // GRBW
}
strip_led_type_t;

Possibly another struct to handle LED strip-level context; the handle that some API functions may use.
Content of this struct is subject to discussion and refinement. Provide suggestions for improvement.
typedef struct
{
    HAL_USART_TypeDef_t *huart;
    uint16_t u16_num_pixels;
    strip_led_type_t e_strip_led_type;
    bool b_strip_update_in_progress;
    bool b_led_pixel_data_malloced;
    bool b_uart_dma_buffer_malloced;
    ws2812_led_pixel_t *p_x_led_pixel_data;
    uint8_t *p_u8_uart_dma_buffer;
    // maybe more members, TBD
    // struct members should be organized for efficient packing and alignment.
}
led_strip_handle_t;

5. EXISTING REFERENCE CODE (FOR CONTEXT ONLY)

These snippets are provided for explanation and background only. You are free (and encouraged) to refactor, improve, or completely redesign them for better API design, performance, maintainability, or abstraction.

Lookup Table:
const uint8_t ws2812_lut[8] =
{
 0x5B, 0x1B, 0x53, 0x13,   // 000 001 010 011
 0x5A, 0x1A, 0x52, 0x12    // 100 101 110 111
};

Current Conversion Function - for illustrative purposes/reference only:

void led_to_uart_dma(const ws2812_led_t *leds,
                     void *p_v_uart_buf, // maybe void * with internal casting?
                     size_t num_leds,
                     led_type_t type)
{
    uint8_t *dst = (uint8_t *) p_v_uart_buf;
    uint32_t pending = 0;
    int pending_bits = 0;
    const int bits_per_led = (int)type;

    for (size_t i = 0; i < num_leds; ++i)
    {
        uint32_t color = ((uint32_t)leds[i].green << 16) |
                         ((uint32_t)leds[i].red   <<  8) |
                          (uint32_t)leds[i].blue;

        if (bits_per_led == 32)
            color = (color << 8) | leds[i].white;   // GRBW

        for (int b = bits_per_led - 1; b >= 0; --b)
        {
            pending = (pending << 1) | ((color >> b) & 1);
            pending_bits++;

            if (pending_bits == 3)
            {
                *dst++ = ws2812_lut[pending];
                pending = 0;
                pending_bits = 0;
            }
        }
    }

    // Flush any leftover partial trio (pad LSBs with 0)
    if (pending_bits > 0)
    {
        pending <<= (3 - pending_bits);
        *dst++ = ws2812_lut[pending];
    }

    // Optional but recommended: 2–4 zero bytes for >50 µs reset pulse
    // *dst++ = 0x00; *dst++ = 0x00; *dst++ = 0x00;
}

6. DESIRED API STYLE AND ARCHITECTURE

The API should be high-level and clean. Example desired functions - subject to discussion and refinement:
- led_strip_handle_t* led_strip_init(USART_HandleTypeDef *huart, led_type_t type, ws2812_led_pixel_t *p_x_pixel_buffer, size_t num_leds, void *p_v_uart_dma_buffer);
  If pointer values are NULL, buffers will be malloc()'d based on num_leds value.
- void led_strip_set_pixel(led_strip_handle_t *p_x_strip, uint16_t u16_pixel, uint8_t u8_red, uint8_t u8_green, uint8_t u8_blue, uint8_t u8_white);
- void led_strip_update(led_strip_handle_t *strip);   // triggers DMA transfer
- Support for multiple strips on different USARTs simultaneously; use of led_strip_handle_t in relevant API functions.

7. CODING STYLE PREFERENCES

- Simplified Barr Group Hungarian notation for variables/symbols; e.g. u8_byte, i_integer, *p_u8_uint8_data, *v_void_ptr, x_name for structs or other derived types, e_enum for enum values, etc. 
  Type names generally end with structtype_t, enum types with enumtype_e
- Allman-style brace formatting (opening brace on its own line)
- Clear, well-commented code. Provide explanatory comments when something non-obvious is being done, such as the WS2812 UART data packing and nonstandard UART configuration.
- The driver must accept a standard STM32Cube USART_HandleTypeDef* so it works with any USART.
- Use of floating-point types (float, double) is strongly discouraged. Discuss with user if use of these would simplify implementation or needed for 3rd-party APIs.
- If hardware-level changes are needed, user should be informed so they can manually edit HW configuration via STM32CubeMX.
  - STM32CubeMX is preferred path for making STM32 core IP's and project configuration changes.
  - Init code generated by CubeMX in /Core should not be modified, unless changes are placed in /* USER CODE ... */ sections.
  - If AI can make changes to .ioc for project, let the user know.

8. FREERTOS (MID-TERM / NICE-TO-HAVE)
- Not required for the initial implementation
- User has novice-level experience with FreeRTOS and STM32Cube OS wrappers. Please provide gentle guidance, examples, and best practices when introducing RTOS elements (tasks, notifications, mutexes for buffer protection, etc.)

- Console and Logging Architecture (Preferred Approach)
  - One dedicated Console Task owns the UART completely and runs the interactive menu system.
  - All other tasks call a single function such as xprintf("format %d", value) — nothing more.
  - xprintf() does printf-style formatting on the caller's stack, puts the result into a fixed-size struct, and sends it to a FreeRTOS queue.
  - The Console Task receives messages from the queue and outputs them to the UART.
    This eliminates contention, avoids malloc in the normal path, and keeps real-time tasks (LED driver) unaffected.
  - Queue uses fixed-size messages (e.g. char message[128]) so no dynamic allocation is required for typical logs.
  - May extend to provide xprintf_long() that accepts a malloc'd buffer - caller malloc()s, console task free()s

9. FUTURE EXPERIMENTATION (LOWER PRIORITY)
- Integration with 2.8" (320x240) ST7789 TFT display (SPI preferred for now; parallel 8080-8/16 as future exploration)
- Add MX25R80 or similar SPI FLASH for strip lighting pattern storage
- Semihosting experiments on the F401 board
- Possible evolution to a more generic LED strip driver (USART, SPI fallback, etc.)
- Advanced peripherals (FMC on H573, etc.) only if relevant later

10. DEVELOPMENT GUIDELINES
- Focus first on a clean, robust, portable USART DMA driver
- Make the code extensible — suggestions for better abstractions, state machines, double-buffering, error handling, etc. are welcome
- Keep development focused on the short-term LED driver goal
