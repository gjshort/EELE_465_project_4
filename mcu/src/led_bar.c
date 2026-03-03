#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "led_bar.h"
#include "utils.h"

// * (P2.7, P2.6, P2.5, P2.4), (P4.7, P4.6, P4.5, P4.4)
//   used for 12 seg. LED bar anodes (RED), (GREEN)
// * P5.0, P5.1, P3.4 
//   used for 12 seg. LED bar cathodes

// Red pins occupying upper half of P2
#define RED_SEL0 P2SEL0
#define RED_SEL1 P2SEL1
#define RED_DIR  P2DIR
#define RED_OUT  P2OUT
static const uint16_t LED_BAR_RED_PINS = 0x00F0;

// Green pins occupying upper half of P4
#define GRN_SEL0 P4SEL0
#define GRN_SEL1 P4SEL1
#define GRN_DIR  P4DIR
#define GRN_OUT  P4OUT
static const uint16_t LED_BAR_GRN_PINS = 0x00F0;

// Segment cathode enable values
#define SEG_0_EN 0
#define SEG_1_EN 1
#define SEG_2_EN 2

/**
 * Init TB2 for use by the LED bar to change
 * which segment is active
 */
void init_led_bar_timer()
{
    // --------- TB2 ----------
    // LED_bar segment change rate
    TB2CTL |= TBCLR;            // Clear timers and dividers
    TB2CTL |= TBSSEL__SMCLK;    // Use SMCLK (1 MHz)
    TB2CTL |= MC__UP;           // Use UP mode for compares

    TB2CTL |= CNTL_0;           // 16-bit counter
    TB2CCR0 = TB2_5_MS;         // 5 ms. - LED_bar update rate

    TB2CCTL0 &= ~CCIFG;         //Clear IRQ flag       
    TB2CCTL0 |= CCIE;           //Enable local interrupt
}

/**
 * Initializes the GPIO ports used for
 * the 12 segment LED bar.
 */
void init_led_bar()
{
    // Init Red Anodes
    RED_SEL0 &= ~LED_BAR_RED_PINS;    // Set to Digital IO
    RED_SEL1 &= ~LED_BAR_RED_PINS;
    RED_DIR |= LED_BAR_RED_PINS;      // Set as output
    RED_OUT &= ~LED_BAR_RED_PINS;     // Clear output

    // Init Green Anodes
    GRN_SEL0 &= ~LED_BAR_GRN_PINS;
    GRN_SEL1 &= ~LED_BAR_GRN_PINS;
    GRN_DIR |= LED_BAR_GRN_PINS;
    GRN_OUT &= ~LED_BAR_GRN_PINS;

    // Init Segment 0 Cathode
    P5SEL0 &= ~BIT0;
    P5SEL1 &= ~BIT0;
    P5DIR |= BIT0;
    P5OUT &= ~BIT0;

    // Init Segment 1 Cathode
    P5SEL0 &= ~BIT1;
    P5SEL1 &= ~BIT1;
    P5DIR |= BIT1;
    P5OUT &= ~BIT1;

    // Init Segment 2 Cathode
    P3SEL0 &= ~BIT4;
    P3SEL1 &= ~BIT4;
    P3DIR |= BIT4;
    P3OUT &= ~BIT4;

    init_led_bar_timer();
}

/**
 * Clears the anodes of both colors
 * for the LED Bar
 */
void led_bar_clear_anodes()
{
    RED_OUT &= ~LED_BAR_RED_PINS;
    GRN_OUT &= ~LED_BAR_GRN_PINS;
}

/**
 * Updates the values on the anode bus
 * for the LED bar. 
 * @param led_bar - main's instance of the LED bar
 *
 * Updates the red/green anodes with the value to be displayed
 * in seg_data at the index pointed to by seg_ptr.
 */
void led_bar_write_anodes(volatile LED_BAR *led_bar)
{
    if(led_bar == NULL)
    {
        return;
    }

    if(led_bar->red == true)
    {
        // Move lower nibble of value to upper nibble of xOUT
        RED_OUT |= (led_bar->seg_data[led_bar->seg_ptr] << 4);
    }
    if(led_bar->green == true)
    {
        GRN_OUT |= (led_bar->seg_data[led_bar->seg_ptr] << 4);
    }
}

/**
 * Activates the proper segment on the LED bar
 * by pulling its cathode low.
 * @param led_bar - main's instance of the LED bar
 */
void led_bar_set_cathode(volatile LED_BAR *led_bar)
{
    if(led_bar == NULL)
    {
        return;
    }

    switch(led_bar->seg_ptr)
    {
    case SEG_0_EN:
        P3OUT |= BIT4;
        P5OUT |= BIT1;
        P5OUT &= ~BIT0;
        break;
    case SEG_1_EN:
        P5OUT |= BIT0;
        P3OUT |= BIT4;
        P5OUT &= ~BIT1;
        break;
    case SEG_2_EN:
        P5OUT |= BIT1;
        P5OUT |= BIT0;
        P3OUT &= ~BIT4;
        break;
    default:
        break;
    }
}

/**
 * Directly updates the segment data
 * @param led_bar - main's instance of the LED bar
 * @param seg0_data - data to be displayed on segment 0 (lower nibble)
 * @param seg1_data - data to be displayed on segment 1 (lower nibble)
 * @param seg2_data - data to be displayed on segment 2 (lower nibble)
 */
void led_bar_update_anode_data(volatile LED_BAR *led_bar, uint8_t seg0_data,
                               uint8_t seg1_data, uint8_t seg2_data)
{
    led_bar->seg_data[0] = seg0_data;
    led_bar->seg_data[1] = seg1_data;
    led_bar->seg_data[2] = seg2_data;
}

/**
 * Cycles through the LED bar's colors
 * @param led_bar - main's instance of and LED bar struct
 *
 *  --> Red -> Yellow -> Green --^
 */
void led_bar_change_color(volatile LED_BAR *led_bar)
{
    if(led_bar->red == true && led_bar->green == false) // Is red
    {
        led_bar->green = true;  // Change to yellow
    }
    else if(led_bar->red == true && led_bar->green == true) // Is yellow
    {
        led_bar->red = false;   // Change to green
    }
    else    // Is green
    {
        led_bar->red = true;    // Change to red
        led_bar->green = false;
    }
}
