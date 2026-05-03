/*
 * debug_menu.c
 *
 * Created on: Apr 26, 2026
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "ansi.h"
#include "menu-api.h"
#include "utils.h"
#include "led_strip_control.h"

void v_app_polling_task(void);

#define DEBUG_MENU_STACK_SIZE   4

/** Wait for @ref led_strip_handle_t::b_transfer_in_progress to clear (ms). */
#define DEBUG_LED_TX_WAIT_MS        (500u)

/** Max wait in @ref led_strip_destroy during LED round-trip test (ms). */
#define DEBUG_LED_DESTROY_WAIT_MS   (500u)

/** Inter-frame delay for strip-1 marquee (also STDIN poll window; ms). */
#define DEBUG_LED_MARQUEE_STEP_MS     (200u)

/******************************************************************************
 *
 ******************************************************************************/

static void v_debug_mcu_reset(void)
{
    printf("Resetting MCU...\r\n");
    v_app_delay_ms(100u);
    HAL_NVIC_SystemReset();
}

/******************************************************************************
 *
 ******************************************************************************/

static void v_debug_show_clocks(void)
{
    uint32_t u32_sysclk_freq = HAL_RCC_GetSysClockFreq();
    uint32_t u32_hclk_freq = HAL_RCC_GetHCLKFreq();
    uint32_t u32_pclk1_freq = HAL_RCC_GetPCLK1Freq();

    printf("SYSCLK freq: %lu\r\n"
           "HCLK freq  : %lu\r\n"
           "PCLK1 freq : %lu\r\n",
           u32_sysclk_freq,
           u32_hclk_freq,
           u32_pclk1_freq);
}

/******************************************************************************
 *
 ******************************************************************************/

static void v_debug_quick_test_1(void)
{
    printf("Quick test function 1\r\n");
}

/******************************************************************************
 *
 ******************************************************************************/

static void v_debug_quick_test_2(void)
{
    printf("Quick test function 2\r\n");
}

/******************************************************************************
 * LED strip — demo pattern (ROM). Wire order is GRB + W byte; WS2812 ignores W
 * on the line; SK6812 uses W. Same RGB (+ W ramp) pattern reused on strips 1–3.
 *
 * Test HW (see menu strings): strip1 = 12 WS2812; strip2 = 8 WS2812 (10 px TX);
 * strip3 = 10 SK6812 RGBW.
 ******************************************************************************/

#define DEBUG_LED_DEMO_WHITE_PCT(i)   ((uint8_t)((255u * (5u + (10u * (i)))) / 100u))

/** 10-pixel demo row — strip 2 (extra 2 bytes harmless on 8-LED line), strip 3 SK6812. */
static const led_rgbw_pixel_t x_debug_led_demo_pattern_10px[10] =
{
    { .u8_green = 0u,   .u8_red = 128u, .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(0) },  /* ~50% red */
    { .u8_green = 64u,  .u8_red = 128u, .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(1) },  /* orange */
    { .u8_green = 128u, .u8_red = 128u, .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(2) },  /* yellow */
    { .u8_green = 128u, .u8_red = 0u,   .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(3) },  /* green */
    { .u8_green = 128u, .u8_red = 0u,   .u8_blue = 128u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(4) },  /* cyan */
    { .u8_green = 0u,   .u8_red = 0u,   .u8_blue = 128u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(5) },  /* blue */
    { .u8_green = 0u,   .u8_red = 128u, .u8_blue = 128u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(6) },  /* magenta */
    { .u8_green = 0u,   .u8_red = 96u,  .u8_blue = 128u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(7) },  /* violet */
    { .u8_green = 64u,  .u8_red = 64u,  .u8_blue = 64u,  .u8_white = DEBUG_LED_DEMO_WHITE_PCT(8) },  /* ~25% grey */
    { .u8_green = 191u, .u8_red = 191u, .u8_blue = 191u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(9) },  /* ~75% grey */
};

/** Strip 1: 12 WS2812 in a circle — first 10 match x_debug_led_demo_pattern_10px, then wrap. */
static const led_rgbw_pixel_t x_debug_led_demo_pattern_12px[12] =
{
    { .u8_green = 0u,   .u8_red = 128u, .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(0) },
    { .u8_green = 64u,  .u8_red = 128u, .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(1) },
    { .u8_green = 128u, .u8_red = 128u, .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(2) },
    { .u8_green = 128u, .u8_red = 0u,   .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(3) },
    { .u8_green = 128u, .u8_red = 0u,   .u8_blue = 128u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(4) },
    { .u8_green = 0u,   .u8_red = 0u,   .u8_blue = 128u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(5) },
    { .u8_green = 0u,   .u8_red = 128u, .u8_blue = 128u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(6) },
    { .u8_green = 0u,   .u8_red = 96u,  .u8_blue = 128u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(7) },
    { .u8_green = 64u,  .u8_red = 64u,  .u8_blue = 64u,  .u8_white = DEBUG_LED_DEMO_WHITE_PCT(8) },
    { .u8_green = 191u, .u8_red = 191u, .u8_blue = 191u, .u8_white = DEBUG_LED_DEMO_WHITE_PCT(9) },
    { .u8_green = 0u,   .u8_red = 128u, .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(0) },  /* wrap */
    { .u8_green = 64u,  .u8_red = 128u, .u8_blue = 0u,   .u8_white = DEBUG_LED_DEMO_WHITE_PCT(1) },
};

#undef DEBUG_LED_DEMO_WHITE_PCT

/** Inter-frame reset tail (UART zero bytes) — same for WS2812 / SK6812 at this line rate. */
#define DEBUG_LED_RESET_TAIL_BYTES      (80u)

#define DEBUG_LED_STRIP1_PIXEL_COUNT    (12u)
#define DEBUG_LED_STRIP2_PIXEL_COUNT    (10u)   /* HW 8 LEDs; two extra logical pixels are harmless */
#define DEBUG_LED_STRIP3_PIXEL_COUNT    (10u)

/** Largest logical count used above (stack off-buffer in @ref v_debug_led_strip_off_impl ). */
#define DEBUG_LED_MAX_PIXEL_COUNT       (12u)

static const char *p_c_led_strip_err_str(led_strip_err_t x_err)
{
    switch (x_err)
    {
        case LED_STRIP_ERR_OK:
            return "OK";

        case LED_STRIP_ERR_NULL:
            return "NULL";

        case LED_STRIP_ERR_PARAM:
            return "PARAM";

        case LED_STRIP_ERR_BUSY:
            return "BUSY";

        case LED_STRIP_ERR_MALLOC:
            return "MALLOC";

        case LED_STRIP_ERR_HAL:
            return "HAL";

        default:
            return "?";
    }
}

/**
 * @brief Demo pattern on @p p_x_uart (ROM pixels, DMA malloc, @p u16_pixels logical LEDs).
 */
static void v_debug_led_strip_on_impl(UART_HandleTypeDef *p_x_uart,
                                      const char *p_c_uart_tag,
                                      led_strip_type_t x_type,
                                      uint16_t u16_pixels,
                                      const led_rgbw_pixel_t *p_x_pattern)
{
    led_strip_handle_t x_handle;
    led_strip_err_t x_err;
    bool b_created;
    uint32_t u32_wait_start_ms;

    printf("Running test on %s\r\n", p_c_uart_tag);

    b_created = false;

    x_err = led_strip_create(&x_handle,
                             p_x_uart,
                             x_type,
                             u16_pixels,
                             (uint16_t) DEBUG_LED_RESET_TAIL_BYTES,
                             (led_rgbw_pixel_t *) p_x_pattern,
                             NULL);

    printf("led_strip_create: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);

    if (x_err != LED_STRIP_ERR_OK)
    {
        return;
    }

    b_created = true;

    x_err = led_strip_update(&x_handle);
    printf("led_strip_update: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);

    if (x_err != LED_STRIP_ERR_OK)
    {
        goto cleanup;
    }

    u32_wait_start_ms = HAL_GetTick();
    while (x_handle.b_transfer_in_progress)
    {
        if (ELAPSED_TIME(u32_wait_start_ms) >= DEBUG_LED_TX_WAIT_MS)
        {
            printf("TX wait: TIMEOUT (still busy after %lu ms)\r\n",
                   (unsigned long) DEBUG_LED_TX_WAIT_MS);
            goto cleanup;
        }
    }

    printf("TX wait: completed\r\n");

cleanup:
    if (b_created)
    {
        x_err = led_strip_destroy(&x_handle, DEBUG_LED_DESTROY_WAIT_MS);
        printf("led_strip_destroy: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);
    }
}

/**
 * @brief All-zero frame on @p p_x_uart (stack buffer up to DEBUG_LED_MAX_PIXEL_COUNT, DMA malloc).
 */
static void v_debug_led_strip_off_impl(UART_HandleTypeDef *p_x_uart,
                                       const char *p_c_uart_tag,
                                       led_strip_type_t x_type,
                                       uint16_t u16_pixels)
{
    led_strip_handle_t x_handle;
    led_strip_err_t x_err;
    bool b_created;
    uint32_t u32_wait_start_ms;
    led_rgbw_pixel_t x_pixels_off[DEBUG_LED_MAX_PIXEL_COUNT] = { 0 };

    if (u16_pixels > DEBUG_LED_MAX_PIXEL_COUNT)
    {
        printf("strip off: bad pixel count %u\r\n", (unsigned) u16_pixels);
        return;
    }

    printf("Running test on %s\r\n", p_c_uart_tag);

    b_created = false;

    x_err = led_strip_create(&x_handle,
                             p_x_uart,
                             x_type,
                             u16_pixels,
                             (uint16_t) DEBUG_LED_RESET_TAIL_BYTES,
                             x_pixels_off,
                             NULL);

    printf("led_strip_create: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);

    if (x_err != LED_STRIP_ERR_OK)
    {
        return;
    }

    b_created = true;

    x_err = led_strip_update(&x_handle);
    printf("led_strip_update: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);

    if (x_err != LED_STRIP_ERR_OK)
    {
        goto cleanup;
    }

    u32_wait_start_ms = HAL_GetTick();
    while (x_handle.b_transfer_in_progress)
    {
        if (ELAPSED_TIME(u32_wait_start_ms) >= DEBUG_LED_TX_WAIT_MS)
        {
            printf("TX wait: TIMEOUT (still busy after %lu ms)\r\n",
                   (unsigned long) DEBUG_LED_TX_WAIT_MS);
            goto cleanup;
        }
    }

    printf("TX wait: completed\r\n");

cleanup:
    if (b_created)
    {
        x_err = led_strip_destroy(&x_handle, DEBUG_LED_DESTROY_WAIT_MS);
        printf("led_strip_destroy: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);
    }
}

static void v_debug_led_strip1_on(void)
{
    v_debug_led_strip_on_impl(&LED_CHANNEL_1_UART_HANDLE,
                              VSTR(LED_CHANNEL_1_UART_HANDLE),
                              LED_STRIP_TYPE_WS2812,
                              (uint16_t) DEBUG_LED_STRIP1_PIXEL_COUNT,
                              x_debug_led_demo_pattern_12px);
}

static void v_debug_led_strip2_on(void)
{
    v_debug_led_strip_on_impl(&LED_CHANNEL_2_UART_HANDLE,
                              VSTR(LED_CHANNEL_2_UART_HANDLE),
                              LED_STRIP_TYPE_WS2812,
                              (uint16_t) DEBUG_LED_STRIP2_PIXEL_COUNT,
                              x_debug_led_demo_pattern_10px);
}

static void v_debug_led_strip3_on(void)
{
    v_debug_led_strip_on_impl(&LED_CHANNEL_3_UART_HANDLE,
                              VSTR(LED_CHANNEL_3_UART_HANDLE),
                              LED_STRIP_TYPE_SK6812,
                              (uint16_t) DEBUG_LED_STRIP3_PIXEL_COUNT,
                              x_debug_led_demo_pattern_10px);
}

static void v_debug_led_strip1_off(void)
{
    v_debug_led_strip_off_impl(&LED_CHANNEL_1_UART_HANDLE,
                               VSTR(LED_CHANNEL_1_UART_HANDLE),
                               LED_STRIP_TYPE_WS2812,
                               (uint16_t) DEBUG_LED_STRIP1_PIXEL_COUNT);
}

static void v_debug_led_strip2_off(void)
{
    v_debug_led_strip_off_impl(&LED_CHANNEL_2_UART_HANDLE,
                               VSTR(LED_CHANNEL_2_UART_HANDLE),
                               LED_STRIP_TYPE_WS2812,
                               (uint16_t) DEBUG_LED_STRIP2_PIXEL_COUNT);
}

static void v_debug_led_strip3_off(void)
{
    v_debug_led_strip_off_impl(&LED_CHANNEL_3_UART_HANDLE,
                               VSTR(LED_CHANNEL_3_UART_HANDLE),
                               LED_STRIP_TYPE_SK6812,
                               (uint16_t) DEBUG_LED_STRIP3_PIXEL_COUNT);
}

/** Marquee spoke levels: R 75%, G 50%, B 25% (WS2812 W unused). */
#define DEBUG_LED_MARQUEE_R     ((uint8_t)((255u * 75u) / 100u))
#define DEBUG_LED_MARQUEE_G     ((uint8_t)((255u * 50u) / 100u))
#define DEBUG_LED_MARQUEE_B     ((uint8_t)((255u * 25u) / 100u))

/**
 * @brief Wait for UART DMA TX idle or timeout; calls v_app_polling_task while spinning.
 */
static bool v_debug_led_wait_tx_done(led_strip_handle_t *p_x_handle)
{
    uint32_t u32_wait_start_ms;

    u32_wait_start_ms = HAL_GetTick();
    while (p_x_handle->b_transfer_in_progress)
    {
        if (ELAPSED_TIME(u32_wait_start_ms) >= DEBUG_LED_TX_WAIT_MS)
        {
            printf("TX wait: TIMEOUT (still busy after %lu ms)\r\n",
                   (unsigned long) DEBUG_LED_TX_WAIT_MS);
            return false;
        }
        v_app_polling_task();
    }

    return true;
}

/**
 * @brief Three LEDs spaced by 3 indices, colors R/G/B at fixed levels; base advances each step.
 *
 * @details Uses @ref i_getchar_blocking_with_timeout between steps. ESC (0x1B) stops the loop.
 *          Caller owns create/destroy; strip length must be >= 3.
 */
static void v_debug_led_marquee_spoke_rgb_spin(led_strip_handle_t *p_x_handle,
                                               uint32_t u32_step_delay_ms)
{
    led_rgbw_pixel_t *p_x;
    uint16_t u16_n;
    uint16_t u16_base;
    int i_key;

    if (p_x_handle == NULL || !p_x_handle->b_initialized || p_x_handle->p_x_pixel == NULL)
    {
        return;
    }

    u16_n = p_x_handle->u16_strip_length;
    if (u16_n < 3u)
    {
        printf("marquee: need strip length >= 3\r\n");
        return;
    }

    p_x = p_x_handle->p_x_pixel;
    u16_base = 0u;
    printf("Marquee: ESC to stop; other keys shorten step wait only.\r\n");

    for (;;)
    {
        led_strip_err_t x_err;
        uint16_t i;

        for (i = 0u; i < u16_n; i++)
        {
            p_x[i].u32_all = 0u;
        }

        {
            uint16_t i_r = (uint16_t) (u16_base % u16_n);
            uint16_t i_g = (uint16_t) ((u16_base + 3u) % u16_n);
            uint16_t i_b = (uint16_t) ((u16_base + 6u) % u16_n);

            p_x[i_r].u8_red = DEBUG_LED_MARQUEE_R;
            p_x[i_r].u8_green = 0u;
            p_x[i_r].u8_blue = 0u;
            p_x[i_r].u8_white = 0u;

            p_x[i_g].u8_red = 0u;
            p_x[i_g].u8_green = DEBUG_LED_MARQUEE_G;
            p_x[i_g].u8_blue = 0u;
            p_x[i_g].u8_white = 0u;

            p_x[i_b].u8_red = 0u;
            p_x[i_b].u8_green = 0u;
            p_x[i_b].u8_blue = DEBUG_LED_MARQUEE_B;
            p_x[i_b].u8_white = 0u;
        }

        x_err = led_strip_update(p_x_handle);
        if (x_err != LED_STRIP_ERR_OK)
        {
            printf("marquee led_strip_update: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);
            break;
        }

        if (!v_debug_led_wait_tx_done(p_x_handle))
        {
            break;
        }

        i_key = i_getchar_blocking_with_timeout(u32_step_delay_ms);
        if (i_key == 0x1B)
        {
            break;
        }

        u16_base = (uint16_t) ((u16_base + 1u) % u16_n);
    }
}

#undef DEBUG_LED_MARQUEE_R
#undef DEBUG_LED_MARQUEE_G
#undef DEBUG_LED_MARQUEE_B

/**
 * @brief Strip 1: create, run marquee, zeros + TX, destroy.
 */
static void v_debug_led_strip1_marquee(void)
{
    led_strip_handle_t x_handle;
    led_strip_err_t x_err;
    bool b_created;
    uint16_t i;
    led_rgbw_pixel_t x_pixels[DEBUG_LED_STRIP1_PIXEL_COUNT] = { 0 };

    printf("Marquee on %s\r\n", VSTR(LED_CHANNEL_1_UART_HANDLE));

    b_created = false;

    x_err = led_strip_create(&x_handle,
                             &LED_CHANNEL_1_UART_HANDLE,
                             LED_STRIP_TYPE_WS2812,
                             (uint16_t) DEBUG_LED_STRIP1_PIXEL_COUNT,
                             (uint16_t) DEBUG_LED_RESET_TAIL_BYTES,
                             x_pixels,
                             NULL);

    printf("led_strip_create: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);

    if (x_err != LED_STRIP_ERR_OK)
    {
        return;
    }

    b_created = true;

    v_debug_led_marquee_spoke_rgb_spin(&x_handle, DEBUG_LED_MARQUEE_STEP_MS);

    for (i = 0u; i < DEBUG_LED_STRIP1_PIXEL_COUNT; i++)
    {
        x_pixels[i].u32_all = 0u;
    }

    x_err = led_strip_update(&x_handle);
    printf("marquee off led_strip_update: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);

    if (x_err == LED_STRIP_ERR_OK)
    {
        (void) v_debug_led_wait_tx_done(&x_handle);
    }

    if (b_created)
    {
        x_err = led_strip_destroy(&x_handle, DEBUG_LED_DESTROY_WAIT_MS);
        printf("led_strip_destroy: %s (%d)\r\n", p_c_led_strip_err_str(x_err), (int) x_err);
    }
}

//------------------------------------------------------------------------------

static const menu_item_t x_led_controller_tests_submenu[] =
{
    {
        .item_type = MENU_ITEM_HELP_TEXT_FIXED,
        .key = 0,
        .text = "\r\n--- LED strip controller tests ---\r\n"
    },
    {
        .item_type = MENU_ITEM_HELP,
        .key = '?',
        .text = NULL
    },
    {
        .item_type = MENU_ITEM_HELP_HIDDEN,
        .key = '\r',
        .text = NULL
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '1',
        .text = "Strip 1 on - WS2812 circle (12 LEDs, CH1 UART, ROM pattern)",
        .function = v_debug_led_strip1_on
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '2',
        .text = "Strip 2 on - WS2812 line (10 px TX, HW 8 LEDs, CH2 UART, ROM pattern)",
        .function = v_debug_led_strip2_on
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '3',
        .text = "Strip 3 on - SK6812 RGBW line (10 LEDs, CH3 UART, ROM pattern)",
        .function = v_debug_led_strip3_on
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '!',
        .text = "Strip 1 off (zeros)",
        .function = v_debug_led_strip1_off
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '@',
        .text = "Strip 2 off (zeros)",
        .function = v_debug_led_strip2_off
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '#',
        .text = "Strip 3 off (zeros)",
        .function = v_debug_led_strip3_off
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '4',
        .text = "Strip 1 marquee - RGB spokes (12 LED, ESC to stop)",
        .function = v_debug_led_strip1_marquee
    },
    {
        .item_type = MENU_ITEM_RETURN_TO_PREVIOUS_MENU,
        .key = 0x1B,
        .text = NULL
    },
    {
        .item_type = MENU_ITEM_END_OF_LIST,
    }
};

//------------------------------------------------------------------------------

static const menu_item_t x_debug_top_menu[] =
{
    {
        .item_type = MENU_ITEM_HELP_TEXT_FIXED,
        .key = 0,
        .text = "\r\n--- LED Strip Controller Test/Debug Main Menu ---\r\n"
    },
    {
        .item_type = MENU_ITEM_HELP,
        .key = '?',
        .text = NULL
    },
    {
        .item_type = MENU_ITEM_HELP_HIDDEN,
        .key = '\r',
        .text = NULL
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '!',
        .text = "MCU RESET",
        .function = v_debug_mcu_reset
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '%',
        .text = "Show system clock frequencies",
        .function = v_debug_show_clocks
    },
    {
        .item_type = MENU_ITEM_CALL_MENU,
        .key = 'x',
        .text = "LED strip controller tests",
        .menu = x_led_controller_tests_submenu
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '1',
        .text = "Quick test function #1",
        .function = v_debug_quick_test_1
    },
    {
        .item_type = MENU_ITEM_FUNCTION,
        .key = '2',
        .text = "Quick test function #2",
        .function = v_debug_quick_test_2
    },
    {
        .item_type = MENU_ITEM_END_OF_LIST,
    }
};

/******************************************************************************
 * Initialize debug menu
 * Typically called from app_main() during system initialization
 ******************************************************************************/

static void *x_debug_menu_stack[DEBUG_MENU_STACK_SIZE];
static menu_control_t x_debug_menu_control;

void v_debug_menu_init(void)
{
    v_menu_init(&x_debug_menu_control,
                x_debug_top_menu,
                &x_debug_menu_stack[0],
                DEBUG_MENU_STACK_SIZE);

    // key param == 0xFF to request help printout
    v_menu_exec(&x_debug_menu_control, 0xFF);
}

/******************************************************************************
 *
 ******************************************************************************/

static void v_debug_menu_exec(char c_key)
{
    if (x_debug_menu_control.menu_stack == NULL)
    {
        v_debug_menu_init();
    }
    v_menu_exec(&x_debug_menu_control, c_key);
}

/******************************************************************************
 * Debug menu service
 * Keeps debug menu active - call this from app_main() infinite loop
 ******************************************************************************/

void v_debug_menu_service(void)
{
    static bool b_reentry_lock;

    if (b_reentry_lock)
    {
        return;
    }
    b_reentry_lock = true;

    int i_key;
    char str_key[4];

    do
    {
        i_key = getchar();
        if (i_key <= 0)
        {
            break;
        }

        p_c_char_to_str((char) i_key, str_key);

        printf("Cmd [%s]\r\n", str_key);
        v_debug_menu_exec((char) i_key);
    }
    while (1);

    b_reentry_lock = false;
}

