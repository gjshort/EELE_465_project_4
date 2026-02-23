#include <msp430fr2153.h>
#include "gpio.h"
#include "utils.h"

/**
 * Inits P3.2 as a GPIO output
 * Used as a heartbeat LED to indicate the 
 * system is oprta
 */
void init_heartbeat_led()
{
    P3SEL0 &= ~BIT2;    // Set to Digital IO
    P3SEL1 &= ~BIT2;

    P3DIR |= BIT2;      // Set as output
    P3OUT &= ~BIT2;     // Clear output
}

/**
 * Inits P3.3 as a GPIO output
 * Used as an LED for indicating the status
 * of the LED_bar in the system.
 */
void init_bar_status_led()
{
    P3SEL0 &= ~BIT3;    // Set to Digital IO
    P3SEL1 &= ~BIT3;

    P3DIR |= BIT3;      // Set as output
    P3OUT &= ~BIT3;     // Clear output
}

/**
 * Inits P3.5 as a GPIO input
 * Used for a button to change the mode 
 * of the LED_bar from pattern to register.
 */
void init_bar_mode_button()
{
    P3SEL0 &= ~BIT5;    // Set to Digital IO
    P3SEL1 &= ~BIT5;

    P3DIR &= ~BIT5;     // Set as input
    P3REN |= BIT5;      // Enable resistor
    P3OUT &= ~BIT5;     // Pull down resistor
}

/**
 * Inits P3.6 as a GPIO input
 * Used for a button to change the state 
 * of the LED_bar. Either a new pattern
 * or a new register depending on the mode.
 */
void init_bar_state_button()
{
    P3SEL0 &= ~BIT6;    // Set to Digital IO
    P3SEL1 &= ~BIT6;

    P3DIR &= ~BIT6;     // Set as input
    P3REN |= BIT6;      // Enable resistor
    P3OUT &= ~BIT6;     // Pull down resistor
}

/**
 * Inits P3.7 as a GPIO input
 * Used for a button to change the color 
 * of the LED_bar.
 */
void init_bar_color_button()
{
    P3SEL0 &= ~BIT7;    // Set to Digital IO
    P3SEL1 &= ~BIT7;

    P3DIR &= ~BIT7;     // Set as input
    P3REN |= BIT7;      // Enable resistor
    P3OUT &= ~BIT7;     // Pull down resistor
}

/**
 * Inits P1.4 as a GPIO input
 * Used for a button to change the color 
 * of the LED_bar.
 */
void init_bar_reset_button()
{
    P1SEL0 &= ~BIT4;    // Set to Digital IO
    P1SEL1 &= ~BIT4;

    P1DIR &= ~BIT4;     // Set as input
    P1REN |= BIT4;      // Enable resistor
    P1OUT &= ~BIT4;     // Pull down resistor
}

/**
 * Inits TB0 as the timer for:
 * Heartbeat LED - 1 sec. on/off
 * LED_bar pattern switch rate - 1 sec. (default)
 */
void init_gpio_timers()
{
    // --------- TB1 ----------
    // 1 sec. duty cycle heartbeat LED
    TB1CTL |= TBCLR;            // Clear timers and dividers
    TB1CTL |= TBSSEL__SMCLK;    // Use SMCLK (1 MHz)
    TB1CTL |= MC__UP;           // Use UP mode for compares

    TB1CTL |= CNTL_0;           // 16-bit counter
    TB1CTL |= ID__4;            // Divide by 4
    TB1EX0 |= TBIDEX__5;        // Divide by 5

    TB1CCR0 = TB1_1_SEC;        // 1 sec. - Heartbeat LED duty cycle

    TB1CCTL0 &= ~CCIFG;         //Clear IRQ flag       
    TB1CCTL0 |= CCIE;           //Enable local interrupt

}

/**
 * Calls all GPIO initialization functions.
 */
void init_gpio()
{
    init_heartbeat_led();
    init_bar_status_led();
    init_bar_mode_button();
    init_bar_state_button();
    init_bar_color_button();
    init_bar_reset_button();
    init_gpio_timers();
}
