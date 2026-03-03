#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "led_pattern.h"
#include "utils.h"

// LED Pattern macros
#define PAT_0_START    0x0AAA   // xxxx |1010 1010 1010
#define PAT_0_TOGGLE   0x0FFF   // xxxx |1111 1111 1111

#define PAT_1_MIN      0x0000
#define PAT_1_MAX      0x0FFF

#define PAT_2_MOVE_OUT 0
#define PAT_2_MOVE_IN  1
#define PAT_2_EDGES    0x0801   // xxxx |1000 0000 0001
#define PAT_2_MIDDLE   0x0060   // xxxx |0000 0110 0000
#define PAT_2_LEFT     0x0FC0   // xxxx |[oooo oo]xx xxxx
#define PAT_2_RIGHT    0x003F   // xxxx |xxxx  xx[oo oooo]

#define PAT_3_MIN      0x0000
#define PAT_3_MAX      0x0FFF

#define PAT_4_RIGHT    0x0001   // xxxx |0000 0000 0001
#define PAT_4_LEFT     0x0800   // xxxx |1000 0000 0000

#define PAT_5_START    0x0800   // xxxx |1000 0000 0000
#define PAT_5_END      0x0FFF   // xxxx |1111 1111 1111


/**
 * Init TB0 to advance LED pattern
 * states
 */
void init_led_pattern_timer()
{
    // --------- TB0 ----------
    // LED_bar pattern state change time (default 1 sec.)
    TB0CTL |= TBCLR;            // Clear timers and dividers
    TB0CTL |= TBSSEL__SMCLK;    // Use SMCLK (1 MHz)
    TB0CTL |= MC__UP;           // Use UP mode for compares

    TB0CTL |= CNTL_0;           // 16-bit counter
    TB0CTL |= ID__4;            // Divide by 4
    TB0EX0 |= TBIDEX__5;        // Divide by 5

    TB0CCR0 = TB0_1_SEC;        // 1 sec. - LED_bar pattern period

    TB0CCTL0 &= ~CCIFG;         //Clear IRQ flag       
    TB0CCTL0 |= CCIE;           //Enable local interrupt
}

/**
 * Updates the state of the currently running LED pattern.
 * @param led_bar_pattern - main's instance of an LED pattern struct
 *
 * Depending on the current pattern specified in led_bar_pattern,
 * the pattern state will be advanced.
 */
void led_pat_change_pattern_state(volatile LED_PATTERN *led_bar_pattern)
{
    switch(led_bar_pattern->pattern_num)
    {
    case 0: // Toggle Bits
    {
        led_bar_pattern->pattern ^= PAT_0_TOGGLE;
        break;
    }
    case 1: // Up Counter
    { 
        if(led_bar_pattern->pattern >= PAT_1_MAX)
        {
            led_bar_pattern->pattern = PAT_1_MIN;
        }
        else
        {
            led_bar_pattern->pattern++;
        }
        break;
    }
    case 2: // Move in/out
    {
        if(led_bar_pattern->pattern == PAT_2_EDGES)         // At edge LEDs
        {
            led_bar_pattern->pattern_dir = PAT_2_MOVE_IN;   // Change dir to inward
        }
        else if(led_bar_pattern->pattern == PAT_2_MIDDLE)   // At middle LEDs
        {
            led_bar_pattern->pattern_dir = PAT_2_MOVE_OUT;  // Change dir to outward
        }

        // Shift bits left and right of middle in opposite directions
        uint16_t left_bits  = (led_bar_pattern->pattern & PAT_2_LEFT);
        uint16_t right_bits = (led_bar_pattern->pattern & PAT_2_RIGHT);
        if(led_bar_pattern->pattern_dir == PAT_2_MOVE_OUT)
        {
            left_bits = left_bits << 1;
            right_bits = right_bits >> 1;
        }
        else if(led_bar_pattern->pattern_dir == PAT_2_MOVE_IN)
        {
            left_bits = left_bits >> 1;
            right_bits = right_bits << 1;
        }
        led_bar_pattern->pattern = left_bits | right_bits;   // Put left and right bits together

        break;
    }
    case 3: // Down counter
    {
        if(led_bar_pattern->pattern <= PAT_3_MIN)
        {
            led_bar_pattern->pattern = PAT_3_MAX;
        }
        else
        {
            led_bar_pattern->pattern--;
        }
        break;
    }
    case 4: // Rotate left
    {
        if(led_bar_pattern->pattern >= PAT_4_LEFT)
        {
            led_bar_pattern->pattern = PAT_4_RIGHT;
        }
        else 
        {
            led_bar_pattern->pattern = led_bar_pattern->pattern << 1;
        }
        break;
    }
    case 5:
    {
        if(led_bar_pattern->pattern >= PAT_5_END)
        {
            led_bar_pattern->pattern = PAT_5_START;
        }
        else 
        {
            // Push 1's right and fill MSb with a 1
            led_bar_pattern->pattern = (led_bar_pattern->pattern >> 1) | PAT_5_START;
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

/**
 * Advances to the next pattern to be displayed
 * on the led bar.
 * @param led_bar_pattern - main's instance of an LED pattern struct
 */
void led_pat_change_pattern(volatile LED_PATTERN* led_bar_pattern)
{
    if(led_bar_pattern->pattern_num >= 5)
    {
        led_bar_pattern->pattern_num = 0;
    }
    else 
    {
        led_bar_pattern->pattern_num++;
    }
    
    //TB0CTL &= ~MC__STOP;    // Stop timer while updating CCR
    switch(led_bar_pattern->pattern_num)
    {
    case 0:
        led_bar_pattern->pattern = PAT_0_START;  // Every other bit = 1
        //TB0CCR0 = TB0_1_SEC;
        break;
    case 1:
        led_bar_pattern->pattern = PAT_1_MIN;  // Start up counter at 0
        //TB0CCR0 = TB0_500_MS;
        break;
    case 2:
        led_bar_pattern->pattern = PAT_2_MIDDLE;
        led_bar_pattern->pattern_dir = PAT_2_MOVE_OUT;
        //TB0CCR0 = TB0_500_MS;
        break;
    case 3:
        led_bar_pattern->pattern = PAT_3_MAX;  // Start down counter at max
        //TB0CCR0 = TB0_250_MS;
        break;
    case 4:
        led_bar_pattern->pattern = PAT_4_RIGHT;  // Far right LED on
        //TB0CCR0 = TB0_750_MS;
        break;
    case 5:
        led_bar_pattern->pattern = PAT_5_START; // Far left LED on
        //TB0CCR0 = TB0_1_SEC;
        break;
    default:
        break;
    }
    //TB0CTL |= MC__UP;      // Restart timer
}
