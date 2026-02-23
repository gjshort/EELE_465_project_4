/***************************************
* Author:   Gabe Story 
* Date:     02.22.2026
* Class:    EELE 465
* Purpose:  Basic code to test ADC on P1.1 | A1 | pin 6
            If pin 6 receives certain voltage values the 
            period for LED bar patterns changes
*************************************************************************************/

#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>

#include "Potentiometer.h"

#include "led_bar.h"
#include "gpio.h"
#include "utils.h"
#include "led_pattern.h"

#define PAT_MODE 0
#define REG_MODE 1

volatile bool change_led_bar_seg = false;
volatile bool update_led_bar_pat_state = false;

LED_TIME time = {0x19, 0x21, 0x14, 0x30, 0x06, 0x26, 0x00};

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;

    // Init P3.2 for status LED
    P3DIR |= BIT2;
    P3OUT &= ~BIT2;

    // Init ADC for A1 | P1.1 | pin 6
    init_ADCpot();

    // Init LED bar
    init_gpio();
    init_led_bar();
    init_led_pattern_timer();

    // Enable interrupts
    __enable_interrupt();

    // ----- Program Variables -----
    volatile LED_BAR led_bar = {.red = true, .green = false, 
                                .seg_data = {0}, 
                                .seg_ptr = 0};

    // Default LED pattern state
    LED_PATTERN led_bar_pattern = { .pattern_num = 0,
                                    .pattern_dir = 0,
                                    .pattern = 0x0AAA };

    uint8_t led_bar_mode = PAT_MODE;

    bool is_pattern_change_button_low = true;
    bool is_mode_change_button_low = true;
    bool is_color_change_button_low = true;

    while(1) {

        // Sample potentiometer
        ADCpot_period();

        // Change LED bar Mode
        if((P3IN & BIT5) != 0 && is_mode_change_button_low == true)
        {
            if(led_bar_mode == PAT_MODE)
            {
                led_bar_mode = REG_MODE;
                P3OUT |= BIT3;  // Set status LED
                time.reg_num = 0;
                update_led_bar_pat_state = false;       // Ensure anodes don't get overwritten
                TB0CCTL0 &= ~CCIE;                      // Disable pattern state timer IRQ      
                led_time_to_anodes(&led_bar, &time);    // Put reg. 0 on LED bar 
            }
            else if(led_bar_mode == REG_MODE)
            {
                led_bar_mode = PAT_MODE;
                P3OUT &= ~BIT3;  // Clear status LED
                // Return to pattern 0
                led_bar_pattern.pattern_num = 5;
                led_pat_change_pattern(&led_bar_pattern);
                TB0CCTL0 |= CCIE;                   // Restart pattern state timer IRQ
            }
            int i;
            for (i = 5000; i > 0; i--) {}
            is_mode_change_button_low = false;
        }
        else if((P3IN & BIT5) == 0 && is_mode_change_button_low == false)
        {
            is_mode_change_button_low = true;
            int i;
            for (i = 5000; i > 0; i--) {}
        }

        // Change Pattern or Register number displayed on LED bar
        if((P3IN & BIT6) != 0 && is_pattern_change_button_low == true)
        {
            if(led_bar_mode == PAT_MODE)
            {
                led_pat_change_pattern(&led_bar_pattern);
                led_bar_pat_to_anodes(&led_bar_pattern, &led_bar);
            }
            else if(led_bar_mode == REG_MODE)
            {
                led_time_change_register(&time);
                led_time_to_anodes(&led_bar, &time);
            }
            int i;
            for (i = 5000; i > 0; i--) {}
            is_pattern_change_button_low = false;
        }
        else if((P3IN & BIT6) == 0 && is_pattern_change_button_low == false)
        {
            is_pattern_change_button_low = true;
            int i;
            for (i = 5000; i > 0; i--) {}
        }

        // Reset LED bar system to display Pattern 0
        if((P1IN & BIT4) != 0)
        {
            led_bar_mode = PAT_MODE;
            P3OUT &= ~BIT3;  // Clear status LED
            // Go to pattern 0
            led_bar_pattern.pattern_num = 5;
            led_pat_change_pattern(&led_bar_pattern);
            TB0CCTL0 |= CCIE;     // Restart pattern state timer IRQ
        }

        // Change LED bar color
        if((P3IN & BIT7) != 0 && is_color_change_button_low == true)
        {
            led_bar_change_color(&led_bar);
            int i;
            for (i = 5000; i > 0; i--) {}
            is_color_change_button_low = false;
        }
        else if((P3IN & BIT7) == 0 && is_color_change_button_low == false)
        {
            is_color_change_button_low = true;
            int i;
            for (i = 5000; i > 0; i--) {}
        }

        // Activate the next LED bar segment
        if(change_led_bar_seg == true)
        {
            led_bar_clear_anodes();
            led_bar_set_cathode(&led_bar);
            led_bar_write_anodes(&led_bar);
            // Activate next segment on the LED bar
            if(led_bar.seg_ptr <= 1) 
            {
                led_bar.seg_ptr++;
            }
            else 
            {
                led_bar.seg_ptr = 0;
            }
            change_led_bar_seg = false;
        }

        // Change pattern state and update LED bar segment data
        if(update_led_bar_pat_state == true)
        {
            led_pat_change_pattern_state(&led_bar_pattern);
            led_bar_pat_to_anodes(&led_bar_pattern, &led_bar);
            update_led_bar_pat_state = false;
        }

    }
    return 0;
}

// Change LED pattern state - TB0
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0() {
    update_led_bar_pat_state = true;
    TB0CCTL0 &= ~CCIFG;     //Clear IRQ flag
}

// Toggle heartbeat LED - TB1
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0() {
    P3OUT ^= BIT2;          // Toggle P3.2 LED
    TB1CCTL0 &= ~CCIFG;     //Clear IRQ flag
}

// Change segment on LED Bar - TB2
#pragma vector = TIMER2_B0_VECTOR
__interrupt void ISR_TB2_CCR0() {
    change_led_bar_seg = true;
    TB2CCTL0 &= ~CCIFG;     //Clear IRQ flag
}
