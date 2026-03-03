/***************************************
* Author:   Gabe Story 
* Date:     02.22.2026
* Class:    EELE 465
* Purpose:  Basic code to test ADC on P5.0 | A8 | pin 37
            If pin 37 receives more than 3V a LED gets toggled.
***************************************************************************/

#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "Potentiometer.h"

ADCpot a;

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;

    // init P3.2 for status LED
    P3DIR |= BIT2;
    P3OUT &= ~BIT2;

    // init ADC for A8 | P5.0 | pin 37
    init_ADCpot();

    while(1) {

        ADCCTL0 |= ADCENC | ADCSC;          // enable & start conversion
        
        while((ADCIFG & ADCIFG0) == 0) {}   // wait for conversion

        a.ADCpot_value = ADCMEM0;           // read ADC value

        // If A8 > 3v turn on LED
        if(a.ADCpot_value > 3613) {
            P3OUT |= BIT2;
        } else {
            P3OUT &= ~BIT2;
        }

    }
    return 0;
}

