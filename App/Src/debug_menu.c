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

#define DEBUG_MENU_STACK_SIZE   4

/******************************************************************************
 *
 ******************************************************************************/

static void v_debug_mcu_reset(void)
{
    printf("Resetting MCU...\r\n");
    HAL_Delay(100);
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

//------------------------------------------------------------------------------

static const menu_item_t x_example_submenu[] =
{
    {
        .item_type = MENU_ITEM_HELP_TEXT_FIXED,
        .key = 0,
        .text = "\r\n--- Example submenu ---\r\n"
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
        .text = "Example submenu",
        .menu = x_example_submenu
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

