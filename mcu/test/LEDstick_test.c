/***********************************
* Author:   Gabe Story
* Date:     02.26.2026
* Class:    EELE 465
* Purpose:  This c file will hold all the policy

            We will be able to set the color for individual
            pixels, and change brightness
**********************************************************************/

#include    <msp430fr2153.h>
#include    "LEDstick.h"

unsigned int LEDpot;

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Setup CLK, SPI
    init_CLK();
    init_SPI();
    init_LEDstick_color_button();

    // P1.1 | pin 6 for A1
    P1SEL1 |= BIT1;
    P1SEL0 |= BIT1;

    // Configure ADC
    ADCCTL0 &= ~ADCSHT;
    ADCCTL0 |= ADCSHT_2;        // conversion cycles = 16
    ADCCTL0 |= ADCON;

    ADCCTL1 |= ADCSSEL_2;       // ADC use SMCLK
    ADCCTL1 |= ADCSHP;          
    
    ADCCTL2 &= ~ADCRES;         // 12-bit resolution
    ADCCTL2 |= ADCRES_2;

    ADCMCTL0 |= ADCINCH_1;      // ADC input channel to A1

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;

    // clear LED stick
    clearStick();

    while(1) {

        if((P3IN & BIT5) != 0) {
            stickColor_change();
        }

        potStick();
        __delay_cycles(800000);

    }

}
